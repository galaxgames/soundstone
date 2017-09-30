#include <soundstone/SoundSystem.hpp>
#include <stack>
#include <cassert>

using namespace soundstone;
using namespace std;


class Semaphore {
    condition_variable _cv;
    mutex _mutex;
    size_t _completed;
    size_t _previous_completed;
public:
    Semaphore();
    void increment();
    void wait();
};


class SamplerProcessor {
    size_t _length;
    unique_ptr<float[]> _data;
    Sampler *_sampler;
    Semaphore *_semaphore;
public:

    void setup(size_t length, Sampler *sampler, Semaphore *semaphore) {
        _length = length;
        _data = unique_ptr<float[]>(new float[length]);
        _sampler = sampler;
        _semaphore = semaphore;
    }

    void process() {
        _sampler->sample(_data.get(), _length);
        _semaphore->increment();
    }

    const float *data() const {
        return _data.get();
    }
};

Semaphore::Semaphore()
    : _completed(0)
    , _previous_completed(0)
{}

void Semaphore::increment() {
    unique_lock<mutex> lock(_mutex);
    ++_completed;
    lock.unlock();
    _cv.notify_all();
}

void Semaphore::wait() {
    unique_lock<mutex> lock(_mutex);
    size_t target = _previous_completed + 1;
    while (_completed < target) {
        _cv.wait(lock);
    }
    ++_previous_completed;
}


SoundSystem::SoundSystem() {
    cubeb_init(&_cubeb, nullptr, nullptr);
    _stream = nullptr;
    _state = CUBEB_STATE_ERROR;
    if (!init_cubeb()) {
        destroy_cubeb();
    }
    _thread_count = 1;
}

void SoundSystem::move_internal(SoundSystem &&other) noexcept {
    lock_guard<mutex> guard(other._data_mutex);
    _cubeb = other._cubeb;
    _stream = other._stream;
    _state = other._state;
    _sample_rate = other._sample_rate;
    _latency = other._latency;
    _samplers = move(other._samplers);
    _data = move(other._data);
}

SoundSystem::SoundSystem(SoundSystem &&other) noexcept {
    move_internal(move(other));
}

SoundSystem &SoundSystem::operator=(SoundSystem &&other) noexcept {
    if (&other != this) {
        move_internal(move(other));
    }
    return *this;
}

SoundSystem::~SoundSystem() {
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
    sampler->setup(_sample_rate);
}

void SoundSystem::remove_sampler(Sampler *sampler) {
    for (size_t i = 0, ilen = _samplers.size(); i < ilen; ++i) {
        Sampler *other = _samplers[i];
        if (other == sampler) {
            _samplers.erase(_samplers.begin() + static_cast<long>(i));
        }
    }
}

void SoundSystem::update(size_t nsamples) {
    unique_ptr<float[]> mixed(new float[nsamples]);
    Semaphore semaphore;
    size_t sampler_count = _samplers.size();
    unique_ptr<SamplerProcessor[]> all_processors(
        new SamplerProcessor[sampler_count]
    );
    std::stack<SamplerProcessor *> processor_stack;

    for (size_t i = 0; i < sampler_count; ++i) {
        SamplerProcessor *processor = &all_processors[i];
        Sampler *sampler = _samplers[i];
        processor->setup(nsamples, sampler, &semaphore);
        processor_stack.push(processor);
    }

    unsigned int threads_running = 0;
    std::vector<thread> threads;
    while (processor_stack.size() > 0) {
        while (threads_running < _thread_count) {

            if (processor_stack.size() > 0) {
                // Start new thread
                SamplerProcessor *processor = processor_stack.top();
                processor_stack.pop();
                thread worker(&SamplerProcessor::process, processor);
                threads.emplace_back(move(worker));
                ++threads_running;
            }
            else {
                // All done processing
                break;
            }
        }

        if (threads_running > 0) {
            semaphore.wait();
            --threads_running;
        }
    }

    // Clean up all threads
    for (thread &worker : threads) {
        worker.join();
    }

    // All samplers have been processed. Now we can mix.
    // Note: Even this could be paralelized. But I'll focus on that later.

    // Clean initial mix buffer
    for (size_t i = 0; i < nsamples; ++i) {
        mixed[i] = 0;
    }

    // Mix everything together
    for (size_t i = 0; i < sampler_count; ++i) {
        mix(mixed.get(), all_processors[i].data(), nsamples);
    }

    // Submit the final mix to the ringbuffer to be consumed by cubeb.
    {
        lock_guard<mutex> lock(_data_mutex);
        _data.produce(mixed.get(), nsamples);
    }


    // Restart the stream if we previously ran out of data
    if (is_ok()
        && (_state == CUBEB_STATE_DRAINED
            || _state == CUBEB_STATE_STOPPED)
    ) {
        cubeb_stream_start(_stream);
    }

}

void SoundSystem::set_thread_count(size_t count) {
    assert(count > 0);
    _thread_count = count;
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
