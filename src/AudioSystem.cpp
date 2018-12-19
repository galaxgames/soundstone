#include <soundstone/AudioSystem.hpp>
#include <algorithm>
#include <cassert>

using namespace soundstone;
using namespace std;

AudioSystem::AudioSystem() {
    cubeb_init(&_cubeb, nullptr, nullptr);
    _state = CUBEB_STATE_ERROR;
    if (!init_cubeb()) {
        destroy_cubeb();
    }
}

void AudioSystem::move_internal(AudioSystem &&other) noexcept {
    swap(_cubeb, other._cubeb);
    swap(_stream, other._stream);
    swap(_state , other._state);
    swap(_sample_rate, other._sample_rate);
    swap(_latency, other._latency);
    swap(_data, other._data);
}

AudioSystem::AudioSystem(AudioSystem &&other) noexcept {
    assert(&other != this);
    lock_guard<mutex> lock(other._data_mutex);
    move_internal(move(other));
}

AudioSystem &AudioSystem::operator=(AudioSystem &&other) noexcept {
    assert(&other != this);
    unique_lock<mutex> lock(_data_mutex, defer_lock);
    unique_lock<mutex> other_lock(other._data_mutex, defer_lock);
    std::lock(lock, other_lock);
    move_internal(move(other));
    return *this;
}

AudioSystem::~AudioSystem() {
    destroy_cubeb();
}

bool AudioSystem::init_cubeb() {
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
    return rv == CUBEB_OK;
}

void AudioSystem::destroy_cubeb() {
    if (_stream != nullptr) {
        cubeb_stream_destroy(_stream);
        _stream = nullptr;
    }
    if (_cubeb != nullptr) {
        cubeb_destroy(_cubeb);
        _cubeb = nullptr;
    }
}

long AudioSystem::data_callback(
    cubeb_stream *stream, void *user_ptr, const void *input_buffer,
    void *output_buffer, long nframes
) {
    AudioSystem *system = reinterpret_cast<AudioSystem *>(user_ptr);
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

void AudioSystem::state_callback(
    cubeb_stream *stream, void *user_ptr, cubeb_state state
) {
    AudioSystem *system = reinterpret_cast<AudioSystem *>(user_ptr);
    system->_state = state;
}


bool AudioSystem::is_ok() const {
    return _cubeb != nullptr && _stream != nullptr;
}

bool AudioSystem::is_steam_ok() const {
    return _state != CUBEB_STATE_ERROR;
}

bool AudioSystem::is_stream_playing() const {
    return _state == CUBEB_STATE_STARTED;
}

bool AudioSystem::is_stream_drained() const {
    return _state == CUBEB_STATE_DRAINED;
}

uint32_t AudioSystem::samples_buffered() const {
    return _data.size();
}

uint32_t AudioSystem::sample_rate() const {
    return _sample_rate;
}

void AudioSystem::update(const float *data, size_t sample_count) {

    {
        lock_guard<mutex> lock(_data_mutex);
        _data.produce(data, sample_count);
    }

    // Restart the stream if we previously ran out of data
    if (is_ok()
        && (_state == CUBEB_STATE_DRAINED
            || _state == CUBEB_STATE_STOPPED)
        ) {
        cubeb_stream_start(_stream);
    }
}



