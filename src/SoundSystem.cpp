#include <soundstone/SoundSystem.hpp>
#include <stack>
#include <cassert>

#include <iostream>

using namespace soundstone;
using namespace std;


SoundSystem::SoundSystem() {
    cubeb_init(&_cubeb, nullptr, nullptr);
    _stream = nullptr;
    _state = CUBEB_STATE_ERROR;
    _sampler_buffer_length = 0;
    if (!init_cubeb()) {
        destroy_cubeb();
    }
    setup_workers(1);
}

void SoundSystem::move_internal(SoundSystem &&other) noexcept {
    _cubeb = other._cubeb;
    _stream = other._stream;

    _state = other._state;
    _sample_rate = other._sample_rate;
    _latency = other._latency;

    _samplers = move(other._samplers);
    _workers = move(other._workers);
    _threads = move(other._threads);
    _sampler_buffers = move(other._sampler_buffers);
    _semaphore = move(other._semaphore);
    _data = move(other._data);
}

SoundSystem::SoundSystem(SoundSystem &&other) noexcept {
    lock_guard<mutex> lock(other._data_mutex);
    move_internal(move(other));
}

SoundSystem &SoundSystem::operator=(SoundSystem &&other) noexcept {
    if (&other != this) {
        unique_lock<mutex> lhs_lock(_data_mutex, defer_lock);
        unique_lock<mutex> rhs_lock(other._data_mutex, defer_lock);
        lock(lhs_lock, rhs_lock);
        move_internal(move(other));
    }
    return *this;
}

SoundSystem::~SoundSystem() {
    // Stop all workers
    setup_workers(0);
    destroy_cubeb();
}

bool SoundSystem::init_cubeb() {
    int rv;

    rv = cubeb_get_preferred_sample_rate(_cubeb, &_sample_rate);
    if (rv != CUBEB_OK) {
        return false;
    }

    cubeb_stream_params output_params;
    output_params.format = CUBEB_SAMPLE_FLOAT32NE;
    output_params.channels = 1;
    output_params.rate = _sample_rate;
    output_params.layout = CUBEB_LAYOUT_UNDEFINED;

    rv = cubeb_get_min_latency(_cubeb, &output_params, &_latency);
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_init(_cubeb, &_stream, "Soundstone Application",
        nullptr, nullptr,
        nullptr, &output_params,
        _latency, data_callback, state_callback, this
    );
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_start(_stream);
    if (rv != CUBEB_OK) {
        return false;
    }

    return true;
}

void SoundSystem::destroy_cubeb() {
    if (_stream != nullptr) {
        cubeb_stream_destroy(_stream);
        _stream = nullptr;
    }
    if (_cubeb != nullptr) {
        cubeb_destroy(_cubeb);
        _cubeb = nullptr;
    }
}

void SoundSystem::setup_workers(size_t count) {
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

bool SoundSystem::is_ok() const {
    return _cubeb != nullptr && _stream != nullptr;
}

bool SoundSystem::is_steam_ok() const {
    return _state != CUBEB_STATE_ERROR;
}

bool SoundSystem::is_stream_playing() const {
    return _state == CUBEB_STATE_STARTED;
}

bool SoundSystem::is_stream_drained() const {
    return _state == CUBEB_STATE_DRAINED;
}

size_t SoundSystem::samples_buffered() const {
    return _data.size();
}

unsigned int SoundSystem::sample_rate() const {
    return _sample_rate;
}

void SoundSystem::add_sampler(Sampler *sampler) {
    _samplers.push_back(sampler);
    unique_ptr<float[]> buffer(new float[_sampler_buffer_length]);
    _sampler_buffers.emplace_back(move(buffer));
    sampler->setup(_sample_rate);
}

void SoundSystem::remove_sampler(Sampler *sampler) {
    for (size_t i = 0, ilen = _samplers.size(); i < ilen; ++i) {
        Sampler *other = _samplers[i];
        if (other == sampler) {
            long it_offset = static_cast<long>(i);
            _samplers.erase(_samplers.begin() + it_offset);
            _sampler_buffers.erase(_sampler_buffers.begin() + it_offset);
        }
    }
}

void SoundSystem::update(size_t nsamples) {
    size_t sampler_count = _samplers.size();
    size_t worker_count = _workers.size();

    // Make sure sampler buffers are big enough
    if (_sampler_buffer_length < nsamples) {
        for (unique_ptr<float[]> &buffer : _sampler_buffers) {
            buffer = unique_ptr<float[]>(new float[nsamples]);
        }
        _sampler_buffer_length = nsamples;
    }

    // Notify all samplers to commit settings
    for (size_t i = 0; i < sampler_count; ++i) {
        _samplers[i]->commit();
    }

    // Distribute work to workers
    size_t worker_load = sampler_count / worker_count;
    size_t remainder = sampler_count % worker_count;
    size_t next_worker_sampler_offset = 0;

    for (size_t i = 0; i < _workers.size(); ++i) {
        size_t worker_sampler_count = worker_load + (i < remainder ? 1 : 0);
        unique_ptr<SamplerWorker> &worker = _workers[i];
        Sampler **samplers = _samplers.data() + next_worker_sampler_offset;

        // TODO: Re-use this vector or use a stack-allocated vla
        std::vector<float *> buffers;
        buffers.reserve(worker_sampler_count);
        for (size_t j = 0; j < worker_sampler_count; ++j) {
            unique_ptr<float[]> &buffer = _sampler_buffers[next_worker_sampler_offset + j];
            buffers.push_back(buffer.get());
        }

        worker->setup(buffers.data(), samplers,  nsamples, worker_sampler_count, &_semaphore);
        next_worker_sampler_offset += worker_sampler_count;
    }

    size_t workers_finished = 0;
    while(workers_finished < worker_count) {
        _semaphore.wait();
        ++workers_finished;
    }

    // The workers take care of mixing all of their sampler's output, but now
    // we need to mix the output of all the workers
    // Note: Even this could be parallized, but it's not that big of a deal
    // since the number of buffers that needs to be mixed is only equal to the
    // thread count. So yeah, not sure if it would be efficient or not.
    float *accumulator_buffer = _workers[0]->mixed_buffer();
    for (size_t i = 1; i < worker_count; ++i) {
        mix(accumulator_buffer, _workers[i]->mixed_buffer(), nsamples);
    }

    // Submit the final mix to the ringbuffer to be consumed by cubeb.
    {
        lock_guard<mutex> lock(_data_mutex);
        _data.produce(accumulator_buffer, nsamples);
    }

    // Restart the stream if we previously ran out of data
    if (is_ok()
        && (_state == CUBEB_STATE_DRAINED
            || _state == CUBEB_STATE_STOPPED)
    ) {
        cubeb_stream_start(_stream);
    }

    //cout << "Goodbye semapohre " << &semaphore << endl;
}

void SoundSystem::set_thread_count(size_t count) {
    assert(count > 0);
    setup_workers(count);
}


void SoundSystem::mix(float *a, const float *b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        float sample_a = a[i];
        float sample_b = b[i];
        a[i] = sample_a + sample_b;
    }
}

long SoundSystem::data_callback(
    cubeb_stream *stream, void *user_ptr, const void *input_buffer,
    void *output_buffer, long nframes
) {
    SoundSystem *system = reinterpret_cast<SoundSystem *>(user_ptr);
    size_t actual_frames;
    {
        lock_guard<mutex> lock(system->_data_mutex);
        actual_frames = min(static_cast<size_t>(nframes), system->_data.size());
        system->_data.consume(
            reinterpret_cast<float *>(output_buffer),
            actual_frames
        );
    }
    return static_cast<long>(actual_frames);
}

void SoundSystem::state_callback(
    cubeb_stream *stream, void *user_ptr, cubeb_state state
) {
    SoundSystem *system = reinterpret_cast<SoundSystem *>(user_ptr);
    system->_state = state;
}
