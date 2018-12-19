#include <soundstone/AudioProcessor.hpp>
#include <stack>
#include <cassert>
#include <iostream>

using namespace soundstone;
using namespace std;


AudioProcessor::AudioProcessor() {
    setup_workers(1);
}

AudioProcessor::~AudioProcessor() {
    // Stop all workers
    setup_workers(0);
}

void AudioProcessor::setup_workers(uint32_t count) {
    assert(_workers.size() == _threads.size());
    if (_workers.size() < count) {
        _workers.reserve(count);
        _threads.reserve(count);
        while(_workers.size() < count) {
            unique_ptr<SamplerWorker> worker(new SamplerWorker);
            thread worker_thread(&SamplerWorker::process, worker.get());
            _workers.emplace_back(move(worker));
            _threads.emplace_back(move(worker_thread));
        }
    }
    else {
        while (_workers.size() > count) {
            unique_ptr<SamplerWorker> &worker = _workers.back();
            thread &worker_thread = _threads.back();
            worker->stop();
            worker_thread.join();
            _threads.pop_back();
            _workers.pop_back();
        }
        _workers.shrink_to_fit();
        _threads.shrink_to_fit();
        assert(_workers.size() == _threads.size());
    }
}



void AudioProcessor::add_sampler(Sampler *sampler) {
    _samplers.push_back(sampler);
    _sampler_graph.add(sampler);
    _is_graph_dirty = true;
}

void AudioProcessor::route_sampler(Sampler *sampler, Sampler *target) {
    _sampler_graph.set_parent(target, sampler);
    _is_graph_dirty = true;
}

void AudioProcessor::route_sampler_to_root(Sampler *sampler) {
    _sampler_graph.attach_to_root(sampler);
    _is_graph_dirty = true;
}

void AudioProcessor::remove_sampler(Sampler *sampler) {
    for (size_t i = 0, ilen = _samplers.size(); i < ilen; ++i) {
        Sampler *other = _samplers[i];
        if (other == sampler) {
            long it_offset = static_cast<long>(i);
            _samplers.erase(_samplers.begin() + it_offset);
        }
    }
    _sampler_graph.remove(sampler);
    _is_graph_dirty = true;
}

enum class SamplerWorkStatus {
    NotStarted,
    Started,
    Done
};

void AudioProcessor::update(float *data, uint32_t nsamples) {
    // Early out if we have no root samplers
    if (_sampler_graph.root().inputs.empty()) {
        fill_n(data, nsamples, 0);
        return;
    }

    if (_is_graph_dirty) {
        _sampler_graph.build(_ordered_samplers);
        _is_graph_dirty = false;
    }

    size_t sampler_count = _samplers.size();
    size_t worker_count = _workers.size();

    // Make sure existing sampler buffers are big enough
    if (_sampler_buffer_length < nsamples) {
        for (unique_ptr<float[]> &buffer : _sampler_buffers) {
            buffer = unique_ptr<float[]>(new float[nsamples]);
        }
        _sampler_buffer_length = nsamples;
    }

    // Create ordered sampler queue
    vector<const GraphNode<Sampler> *> ordered_samplers;
    vector<SamplerWorkStatus> statuses;
    vector<const GraphNode<Sampler> *> ready_samplers;
    ordered_samplers.insert(ordered_samplers.end(), _ordered_samplers.begin(), _ordered_samplers.end());
    statuses.insert(statuses.end(), ordered_samplers.size(), SamplerWorkStatus::NotStarted);

    // Create new buffers if needed
    while (_sampler_buffers.size() < ordered_samplers.size()) {
        _sampler_buffers.emplace_back(new float[nsamples]);
    }

    // Notify all samplers to commit settings
    for (size_t i = 0; i < sampler_count; ++i) {
        _samplers[i]->commit();
    }

//
//    // Distribute work to workers
//    size_t worker_load = sampler_count / worker_count;
//    size_t remainder = sampler_count % worker_count;
//    size_t next_worker_sampler_offset = 0;
//
//    for (size_t i = 0; i < _workers.size(); ++i) {
//        size_t worker_sampler_count = worker_load + (i < remainder ? 1 : 0);
//        unique_ptr<SamplerWorker> &worker = _workers[i];
//        Sampler **samplers = _samplers.data() + next_worker_sampler_offset;
//
//        // TODO: Re-use this vector or use a stack-allocated vla
//        std::vector<float *> buffers;
//        buffers.reserve(worker_sampler_count);
//        for (size_t j = 0; j < worker_sampler_count; ++j) {
//            unique_ptr<float[]> &buffer = _sampler_buffers[next_worker_sampler_offset + j];
//            buffers.push_back(buffer.get());
//        }
//
//        worker->setup(buffers.data(), samplers,  nsamples, worker_sampler_count, &_semaphore);
//        next_worker_sampler_offset += worker_sampler_count;
//    }

    for (unique_ptr<SamplerWorker> &worker : _workers) {
        worker->reset();
        _semaphore.return_worker(worker.get());
    }
    for (;;) {
        SamplerWorker *worker = _semaphore.wait();

        if (worker->sampler_node() != nullptr) {
            statuses[worker->sampler_node()->order_list_index] = SamplerWorkStatus::Done;
        }

        if (ready_samplers.empty()) {
            // Figure out what samplers to work on next
            bool is_finished = true;
            for (const GraphNode<Sampler> *node : ordered_samplers) {
                if (node == nullptr) {
                    continue;
                }
                int dependency_index = node->dependency_index;
                SamplerWorkStatus status = statuses[node->order_list_index];
                if (status != SamplerWorkStatus::NotStarted) {
                    continue;
                }
                is_finished = false;
                if (dependency_index == -1 || statuses[dependency_index] == SamplerWorkStatus::Done) {
                    // Dependency met, this node is ready
                    statuses[node->order_list_index] = SamplerWorkStatus::Started;
                    ready_samplers.push_back(node);
                }
            }

            if (is_finished) {
                break;
            }
        }
        if (!ready_samplers.empty()) {
            const GraphNode<Sampler> *node = ready_samplers.back();
            ready_samplers.pop_back();

            size_t input_count = node->inputs.size();
            unique_ptr<const float *[]> input_buffers(new const float *[input_count]);
            for (size_t i = 0; i < input_count; ++i) {
                GraphNode<Sampler> *input_node = node->inputs[i].lock().get();
                input_buffers[i] = _sampler_buffers[input_node->order_list_index].get();
            }
            float *output_buffer = _sampler_buffers[node->order_list_index].get();
            worker->setup(input_buffers.get(), output_buffer, node, input_count, nsamples, &_semaphore);
        }

    }

    // Wait for the remaining workers to finish
    size_t workers_finished = 1;
    while(workers_finished < worker_count) {
        _semaphore.wait();
        ++workers_finished;
    }

    // The workers take care of mixing all of their sampler's output, but now
    // we need to mix the output of all the workers
    // Note: Even this could be parallized, but it's not that big of a deal
    // since the number of buffers that needs to be mixed is only equal to the
    // thread count. So yeah, not sure if it would be efficient or not.
    for (size_t i = 0; i < nsamples; ++i) {
        data[i] = 0;
    }

    const GraphNode<Sampler> &root = _sampler_graph.root();
    for (const auto &input : root.inputs) {
        const GraphNode<Sampler> *right_node = input.lock().get();
        float *right_buffer = _sampler_buffers[right_node->order_list_index].get();
        mix(data, right_buffer, nsamples);
    }
}

void AudioProcessor::set_thread_count(uint32_t count) {
    assert(count > 0);
    setup_workers(count);
}

void AudioProcessor::mix(float *a, const float *b, uint32_t count) {
    for (size_t i = 0; i < count; ++i) {
        float sample_a = a[i];
        float sample_b = b[i];
        a[i] = sample_a + sample_b;
    }
}