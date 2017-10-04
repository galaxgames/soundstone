#include <soundstone/RealTimeSoundSystemManager.hpp>

using namespace soundstone;
using namespace std;

RealTimeSoundSystemManager::RealTimeSoundSystemManager(SoundSystem *system)
    : RealTimeSoundSystemManager(system, 1, 8)
{
}

RealTimeSoundSystemManager::RealTimeSoundSystemManager(
    SoundSystem *system, size_t min_buffer_length, size_t max_buffer_length
) {
    _system = system;
    set_buffer_limits(min_buffer_length, max_buffer_length);
}

void RealTimeSoundSystemManager::set_buffer_limits(
    size_t min_buffer_length, size_t max_buffer_length
) {
    _min_buffer_length = min_buffer_length;
    _max_buffer_length = max_buffer_length;
}

void RealTimeSoundSystemManager::update(double elapsed_seconds) {
    double sample_rate = static_cast<double>(_system->sample_rate());
    double samples = sample_rate * elapsed_seconds;
    size_t nsamples = static_cast<size_t>(samples);
    size_t samples_buffered = _system->samples_buffered();
    size_t min_samples_target = (size_t(44100) / size_t(60)) * _min_buffer_length;
    size_t max_samples_target = (size_t(44100) / size_t(60)) * _max_buffer_length;
    if (nsamples + samples_buffered < min_samples_target) {
        nsamples = min_samples_target - samples_buffered;
    }
    else if (nsamples + samples_buffered > max_samples_target) {
        nsamples = max_samples_target - samples_buffered;
    }

    _system->update(nsamples);
}
