#pragma once
#include "AudioProcessor.hpp"
#include "AudioSystem.hpp"
#include <cstddef>

namespace soundstone {

    class RealTimeAudioManager {
        std::size_t _min_buffer_length;
        std::size_t _max_buffer_length;
        AudioSystem *_system;
        AudioProcessor *_processor;
        std::unique_ptr<float[]> _buffer;
        size_t _buffer_length;
    public:

        RealTimeAudioManager(AudioProcessor *processor, AudioSystem *system);
        RealTimeAudioManager(
            AudioProcessor *processor,
            AudioSystem *system,
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
