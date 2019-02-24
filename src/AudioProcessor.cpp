#include <soundstone/AudioProcessor.hpp>
#include <stack>
#include <cassert>
#include <iostream>

using namespace soundstone;
using namespace std;


AudioProcessor::RoutePredicate::RoutePredicate(soundstone::AudioProcessor *processor, soundstone::Module *source) {
    _processor = processor;
    _source = source;
}

void AudioProcessor::RoutePredicate::to(soundstone::Module *destination, uint32_t index) {
    _processor->set_input(destination, index, _source);
}

void AudioProcessor::add_module(Module *module) {
    lock_guard<mutex> lock(_actions_mutex);
    AddRemoveData data;
    data.module = module;
    Action action = { ActionType::ADD_MODULE, { data } };
    _actions.push(action);
}

void AudioProcessor::remove_module(Module *module) {
    lock_guard<mutex> lock(_actions_mutex);
    AddRemoveData data;
    data.module = module;
    Action action = { ActionType::REMOVE_MODULE, { data } };
    _actions.push(action);
}

void AudioProcessor::set_input(Module *module, uint32_t index, Module *input) {
    assert(index < MAX_MODULE_INPUTS);
    lock_guard<mutex> lock(_actions_mutex);
    RouteData data;
    data.source = input;
    data.dest = module;
    data.index = index;
    ActionData action_data = {};
    action_data.route = data;
    Action action = { ActionType::ROUTE_MODULE, action_data };
    _actions.push(action);
}

AudioProcessor::RoutePredicate AudioProcessor::route(soundstone::Module *module) {
    return RoutePredicate(this, module);
}

void AudioProcessor::update(uint32_t nsamples) {
    process_actions();

    if (_dependency_indices_are_dirty) {
        _dependency_indices.clear();
        _dependency_indices.reserve(_harnesses.size());

        for (uint32_t i = 0, ilen = _harnesses.size(); i < ilen; ++i) {
            vector<uint32_t> indices_vector;
            for (Module *sampler : _harnesses[i].inputs) {
                if (sampler != nullptr) {
                    indices_vector.push_back(_modules_to_harnesses[sampler]);
                }
            }

            auto indices = unique_ptr<uint32_t[]>(new uint32_t[indices_vector.size()]);
            copy_n(indices_vector.data(), indices_vector.size(), indices.get());

            _dependency_indices.emplace_back(move(indices), indices_vector.size());
        }

        _dependency_indices_are_dirty = false;
    }

    // Make sure existing module buffers are big enough
    if (_sampler_buffer_length < nsamples) {
        _sampler_buffers.clear();
        _sampler_buffer_length = nsamples;
        _null_sample_buffer = unique_ptr<float[]>(new float[nsamples]{0});
    }

    // Create new buffers if needed
    while (_sampler_buffers.size() < _harnesses.size()) {
        _sampler_buffers.emplace_back(new float[nsamples]);
    }

    // Ensure input vectors are allocated
    while (_input_vectors.size() < _harnesses.size()) {
        _input_vectors.emplace_back();
    }

    // Notify all samplers to commit settings
    for (ModuleHarness &harness : _harnesses) {
        harness.module->commit();
    }

    // Set up all worker functions.
    for (size_t i = 0, ilen = _harnesses.size(); i < ilen; ++i) {
        ModuleHarness &harness = _harnesses[i];
        Module *sampler = harness.module;

        array<const float *, MAX_MODULE_INPUTS> &input_vector = _input_vectors[i];
        const float * const *sampler_inputs = input_vector.data();
        float *output_buffer = _sampler_buffers[i].get();

        for (uint32_t input_index = 0; input_index < MAX_MODULE_INPUTS; ++input_index) {
            Module *input_sampler = harness.inputs[input_index];
            const float *input_buffer;
            if (input_sampler == nullptr) {
                input_buffer = _null_sample_buffer.get();
            } else {
                uint32_t sampler_index = _modules_to_harnesses[input_sampler];
                input_buffer = _sampler_buffers[sampler_index].get();
            }
            input_vector[input_index] = input_buffer;
        }

        unique_ptr<uint32_t[]> &indices = _dependency_indices[i].first;
        uint32_t index_count = _dependency_indices[i].second;

        _party.add_work(
            [=]{sampler->sample(sampler_inputs, output_buffer, nsamples);},
            indices.get(), index_count
        );
    }

    // Do the work.
    _party.work();
}

void AudioProcessor::set_thread_count(uint32_t count) {
    assert(count > 0);
    _party.setup(count);
}


void AudioProcessor::process_actions() {
    lock_guard<mutex> lock(_actions_mutex);

    while (!_actions.empty()) {
        Action action = _actions.front();
        _actions.pop();

        switch (action.type) {
            case ActionType::ADD_MODULE:
                process_add(action.data.add_remove);
                break;
            case ActionType::REMOVE_MODULE:
                process_remove(action.data.add_remove);
                break;
            case ActionType::ROUTE_MODULE:
                process_route(action.data.route);
                break;
        }
    }
}

void AudioProcessor::process_add(AudioProcessor::AddRemoveData data) {
    Module *module = data.module;

    auto it = _modules_to_harnesses.find(module);
    if (it != _modules_to_harnesses.end()) {
        // Module already added.
        return;
    }

    uint32_t index = _harnesses.size();
    _harnesses.emplace_back();
    _harnesses.back().module = module;

    auto result = _modules_to_harnesses.emplace(
        piecewise_construct,
        forward_as_tuple(module),
        forward_as_tuple(index)
    );

    _dependency_indices_are_dirty = true;
}

void AudioProcessor::process_remove(AudioProcessor::AddRemoveData data) {
    Module *module = data.module;

    auto it = _modules_to_harnesses.find(module);
    if (it == _modules_to_harnesses.end()) {
        // Module not present
        return;
    }

    uint32_t index = it->second;
    _modules_to_harnesses.erase(it);

    // Update indices
    for (uint32_t i = index + 1; i < _harnesses.size(); ++i) {
        Module *target = _harnesses[i].module;
        _modules_to_harnesses[target] = i - 1;
    }

    _harnesses.erase(_harnesses.begin() + index);
    _dependency_indices_are_dirty = true;


    // Unset this module as the input of any samplers
    for (ModuleHarness &harness : _harnesses) {
        for (Module *&input : harness.inputs) {
            if (input == module) {
                input = nullptr;
            }
        }
    }
}

void AudioProcessor::process_route(AudioProcessor::RouteData data) {
    assert(data.index < MAX_MODULE_INPUTS);

    auto it = _modules_to_harnesses.find(data.dest);
    if (it == _modules_to_harnesses.end()) {
        // Module has not been added, cannot set input.
        return;
    }

    uint32_t harness_index = it->second;
    ModuleHarness &harness = _harnesses[harness_index];

    harness.inputs[data.index] = data.source;

    _dependency_indices_are_dirty = true;
}
