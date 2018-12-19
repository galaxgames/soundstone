#pragma once
#include "AudioProcessor.hpp"
#include "AudioSystem.hpp"
#include <cstddef>
#include <soundstone/export.h>

namespace soundstone {

    class SOUNDSTONE_EXPORT RealTimeAudioManager {
        uint32_t _min_buffer_length;
        uint32_t _max_buffer_length;
        AudioSystem *_system;
        AudioProcessor *_processor;
        std::unique_ptr<float[]> _buffer;
        size_t _buffer_length;
    public:

        RealTimeAudioManager(AudioProcessor *processor, AudioSystem *system);
        RealTimeAudioManager(
            AudioProcessor *processor,
            AudioSystem *system,
            uint32_t min_buffer_length,
            uint32_t max_buffer_length
        );

        void set_buffer_limits(
            uint32_t min_buffer_length,
            uint32_t max_buffer_length
        );

        void update(double elapsed_seconds);
    };
}
