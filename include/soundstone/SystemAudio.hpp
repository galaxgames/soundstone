#pragma once
#include <soundstone/export.h>
#include "RingBuffer.hpp"

#include <mutex>
#include <functional>

namespace soundstone {

    class SOUNDSTONE_EXPORT SystemAudio {
        class Internal;

        std::unique_ptr<Internal> _internal;
        mutable std::mutex _data_mutex;
        uint32_t _sample_rate = 0;
        uint32_t _latency = 0;
        RingBuffer<float> _data;

        std::function<void()> _drained_callback;
        std::mutex _drained_callback_mutex;

        std::mutex _stream_state_mutex;

        bool init_cubeb();
        void destroy_cubeb();

    public:
        SystemAudio();
        ~SystemAudio();

        bool is_ok() const;
        bool is_steam_ok() const;
        bool is_stream_playing() const;
        bool is_stream_drained() const;
        uint32_t samples_buffered() const;
        uint32_t sample_rate() const;
        uint32_t latency() const;

        void update(const float *data, size_t sample_count);
        void set_drained_callback(std::function<void()> callback);

    };
}