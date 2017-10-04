#pragma once
#include "SoundSystem.hpp"
#include <cstddef>

namespace soundstone {

    class RealTimeSoundSystemManager {
        std::size_t _min_buffer_length;
        std::size_t _max_buffer_length;
        SoundSystem *_system;
    public:

        RealTimeSoundSystemManager(SoundSystem *system);
        RealTimeSoundSystemManager(
            SoundSystem *system,
            size_t min_buffer_length,
            size_t max_buffer_length
        );

        void set_buffer_limits(
            size_t min_buffer_length,
            size_t max_buffer_length
        );

        void update(double elapsed_seconds);
    };
}
