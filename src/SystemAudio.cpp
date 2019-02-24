#include <soundstone/SystemAudio.hpp>

#include <cubeb/cubeb.h>
#include <algorithm>
#include <cassert>

using namespace soundstone;
using namespace std;

namespace soundstone {
    class SystemAudio::Internal {
    public:
        cubeb *cubeb = nullptr;
        cubeb_stream *stream = nullptr;
        cubeb_state state = {};

        static long data_callback(
            cubeb_stream *stream, void *user_ptr, void const *input_buffer,
            void *output_buffer, long nframes
        );

        static void state_callback(
            cubeb_stream *stream, void *user_ptr, cubeb_state state
        );
    };

}


SystemAudio::SystemAudio()
    : _internal(new Internal())
{
    cubeb_init(&_internal->cubeb, nullptr, nullptr);
    _internal->state = CUBEB_STATE_ERROR;
    if (!init_cubeb()) {
        destroy_cubeb();
    }
}

SystemAudio::~SystemAudio() {
    destroy_cubeb();
}

bool SystemAudio::init_cubeb() {
    int rv;

    rv = cubeb_get_preferred_sample_rate(_internal->cubeb, &_sample_rate);
    if (rv != CUBEB_OK) {
        return false;
    }

    cubeb_stream_params output_params;
    output_params.format = CUBEB_SAMPLE_FLOAT32NE;
    output_params.channels = 1;
    output_params.rate = _sample_rate;
    output_params.layout = CUBEB_LAYOUT_UNDEFINED;

    rv = cubeb_get_min_latency(_internal->cubeb, &output_params, &_latency);
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_init(_internal->cubeb, &_internal->stream, "Soundstone Application",
                           nullptr, nullptr,
                           nullptr, &output_params,
                           _latency, Internal::data_callback, Internal::state_callback, this
    );
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_start(_internal->stream);
    return rv == CUBEB_OK;
}

void SystemAudio::destroy_cubeb() {
    if (_internal->stream != nullptr) {
        cubeb_stream_destroy(_internal->stream);
        _internal->stream = nullptr;
    }
    if (_internal->cubeb != nullptr) {
        cubeb_destroy(_internal->cubeb);
        _internal->cubeb = nullptr;
    }
}

long SystemAudio::Internal::data_callback(
    cubeb_stream *stream, void *user_ptr, const void *input_buffer,
    void *output_buffer, long nframes
) {
    SystemAudio *system = reinterpret_cast<SystemAudio *>(user_ptr);
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

void SystemAudio::Internal::state_callback(
    cubeb_stream *stream, void *user_ptr, cubeb_state state
) {
    SystemAudio *system = reinterpret_cast<SystemAudio *>(user_ptr);

    unique_lock<mutex> state_lock(system->_stream_state_mutex);
    system->_internal->state = state;
    state_lock.unlock();

    if (state == CUBEB_STATE_DRAINED) {
        uint32_t buffered_samples;
        { lock_guard<mutex> data_lock(system->_data_mutex);
            buffered_samples = system->_data.size();
        }

        if (buffered_samples > 0) {
            // We've got some data queued up at this point, so lets restart the stream now.
            cubeb_stream_start(stream);
        }

        { lock_guard<mutex> lock(system->_drained_callback_mutex);
            if (system->_drained_callback != nullptr) {
                system->_drained_callback();
            }
        }
    }
}


bool SystemAudio::is_ok() const {
    return _internal->cubeb != nullptr && _internal->stream != nullptr;
}

bool SystemAudio::is_steam_ok() const {
    return _internal->state != CUBEB_STATE_ERROR;
}

bool SystemAudio::is_stream_playing() const {
    return _internal->state == CUBEB_STATE_STARTED;
}

bool SystemAudio::is_stream_drained() const {
    return _internal->state == CUBEB_STATE_DRAINED;
}

uint32_t SystemAudio::samples_buffered() const {
    lock_guard<mutex> lock(_data_mutex);
    return _data.size();
}

uint32_t SystemAudio::sample_rate() const {
    return _sample_rate;
}

uint32_t SystemAudio::latency() const {
    return _latency;
}

void SystemAudio::update(const float *data, size_t sample_count) {
    { lock_guard<mutex> lock(_data_mutex);
        _data.produce(data, sample_count);
    }

    // Restart the stream if we previously ran out of data
    if (is_ok()) {
        bool should_restart_stream;
        { lock_guard<mutex> lock(_stream_state_mutex);
            should_restart_stream = _internal->state == CUBEB_STATE_DRAINED;
        }
        if (should_restart_stream) {
            cubeb_stream_start(_internal->stream);
        }
    }
}

void SystemAudio::set_drained_callback(std::function<void()> callback) {
    lock_guard<mutex> lock(_drained_callback_mutex);
    _drained_callback = move(callback);
}
