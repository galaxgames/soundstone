#pragma once
#include <cubeb/cubeb.h>
#include <mutex>
#include "RingBuffer.hpp"

namespace soundstone {
    class AudioSystem {
        cubeb *_cubeb;
        cubeb_stream *_stream;
        cubeb_state _state;
        std::mutex _data_mutex;
        unsigned int _sample_rate;
        unsigned int _latency;
        RingBuffer<float> _data;

        void move_internal(AudioSystem &&other) noexcept;
        bool init_cubeb();
        void destroy_cubeb();

        static long data_callback(
            cubeb_stream *stream, void *user_ptr, void const *input_buffer,
            void *output_buffer, long nframes
        );

        static void state_callback(
            cubeb_stream *stream, void *user_ptr, cubeb_state state
        );

    public:
        AudioSystem();
        AudioSystem(const AudioSystem &other) = delete;
        AudioSystem(AudioSystem &&other) noexcept;
        AudioSystem &operator=(const AudioSystem &other) = delete;
        AudioSystem &operator=(AudioSystem &&other) noexcept;
        ~AudioSystem();

        bool is_ok() const;
        bool is_steam_ok() const;
        bool is_stream_playing() const;
        bool is_stream_drained() const;
        size_t samples_buffered() const;
        unsigned int sample_rate() const;

        void update(const float *data, size_t sample_count);

    };
}