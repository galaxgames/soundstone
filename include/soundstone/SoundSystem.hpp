#pragma once
#include <vector>
#include <thread>
#include "Sampler.hpp"
#include <cubeb/cubeb.h>
#include <soundstone/RingBuffer.hpp>

namespace soundstone {
    class SoundSystem {

        cubeb *_cubeb;
        cubeb_stream *_stream;
        cubeb_state _state;
        size_t _thread_count;

        unsigned int _sample_rate;
        unsigned int _latency;
        std::vector<Sampler *> _samplers;
        RingBuffer<float> _data;
        std::mutex _data_mutex;

        void move_internal(SoundSystem &&other) noexcept;
        bool init_cubeb();
        void destroy_cubeb();

        static long data_callback(
            cubeb_stream *stream, void *user_ptr, void const *input_buffer,
            void *output_buffer, long nframes
        );

        static void state_callback(
            cubeb_stream *stream, void *user_ptr, cubeb_state state
        );

        /**
         * @brief mix Mix stream b into stream a
         * @param a
         * @param b
         * @param size
         */
        void mix(float *a, const float *b, size_t size);

    public:
        SoundSystem();
        SoundSystem(const SoundSystem &) = delete;
        SoundSystem(SoundSystem &&) noexcept;
        ~SoundSystem();
        SoundSystem &operator=(const SoundSystem &) = delete;
        SoundSystem &operator=(SoundSystem &&) noexcept;

        bool is_ok() const;
        bool is_steam_ok() const;
        bool is_stream_playing() const;
        bool is_stream_drained() const;
        size_t samples_buffered() const;
        unsigned int sample_rate() const;
        void add_sampler(Sampler *sampler);
        void remove_sampler(Sampler *sampler);
        void update(size_t nsamples);
        void set_thread_count(size_t count);

    };
}
