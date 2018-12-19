#include <soundstone/RealTimeAudioManager.hpp>

using namespace soundstone;
using namespace std;

RealTimeAudioManager::RealTimeAudioManager(
    AudioProcessor *processor,
    AudioSystem *system
)
    : RealTimeAudioManager(processor, system, 1, 8)
{
}

RealTimeAudioManager::RealTimeAudioManager(
    AudioProcessor *processor,
    AudioSystem *system,
    uint32_t min_buffer_length,
    uint32_t max_buffer_length
) {
    _buffer_length = 0;
    _processor = processor;
    _system = system;
    set_buffer_limits(min_buffer_length, max_buffer_length);
}

void RealTimeAudioManager::set_buffer_limits(
    uint32_t min_buffer_length, uint32_t max_buffer_length
) {
    _min_buffer_length = min_buffer_length;
    _max_buffer_length = max_buffer_length;
}

void RealTimeAudioManager::update(double elapsed_seconds) {
    uint32_t sample_rate = _system->sample_rate();
    double samples = static_cast<double>(sample_rate) * elapsed_seconds;
    uint32_t nsamples = static_cast<uint32_t>(samples);
    uint32_t samples_buffered = _system->samples_buffered();
    uint32_t min_samples_target = (sample_rate / 60) * _min_buffer_length;
    uint32_t max_samples_target = (sample_rate / 60) * _max_buffer_length;
    if (nsamples + samples_buffered < min_samples_target) {
        nsamples = min_samples_target - samples_buffered;
    }
    else if (nsamples + samples_buffered > max_samples_target) {
        nsamples = max_samples_target - samples_buffered;
    }

    if (nsamples > _buffer_length) {
        _buffer = unique_ptr<float[]>(new float[nsamples]);
        _buffer_length = nsamples;
    }

    _processor->update(_buffer.get(), nsamples);
    _system->update(_buffer.get(), nsamples);
}
