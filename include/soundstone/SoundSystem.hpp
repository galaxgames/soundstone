#pragma once
#include <vector>
#include "Sampler.hpp"
#include <cubeb/cubeb.h>

namespace soundstone {
    class SoundSystem {
        cubeb *_cubeb;
        cubeb_stream *_stream;
        unsigned int _sample_rate;
        unsigned int _latency;
        std::vector<Sampler *> _samplers;

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

        void add_sampler(Sampler *sampler);
        void remove_sampler(Sampler *sampler);

        void update();

    };
}
