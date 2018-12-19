#include "SinewaveSampler.hpp"
#include <cmath>

using namespace soundstone_example;
using namespace soundstone;
using namespace std;

SinewaveSampler::SinewaveSampler(uint32_t frequency, uint32_t sample_rate) {
    _frequency = frequency;
    _i = 0;

    _samples_per_cycle = sample_rate / _frequency;
    _samples = unique_ptr<float[]>(new float[_samples_per_cycle]);
    for (size_t i = 0; i < _samples_per_cycle; ++i) {
        float t = (float)i / (float)_samples_per_cycle;
        _samples[i] = sin(t * static_cast<float>(M_PI) * 2.0f);
    }
}

void SinewaveSampler::commit() {

}

uint32_t SinewaveSampler::sample(
    const float **input_data,
    float *output_data,
    uint32_t input_count,
    uint32_t nsamples
) {
    for (size_t i = 0; i < nsamples; ++i) {
        output_data[i] = _samples[_i];
        _i = (_i + 1) % _samples_per_cycle;
    }
    return nsamples;
}
