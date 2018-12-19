#include "SquareSampler.hpp"

using namespace soundstone;
using namespace soundstone_example;
using namespace std;

SquareSampler::SquareSampler(float frequency, uint32_t sample_rate) {
    _i = 0;
    _frequency = frequency;
    _amplitude = 1;
    _samples_per_cycle = static_cast<uint32_t>(sample_rate / _frequency);
}

void SquareSampler::commit() {

}

uint32_t SquareSampler::sample(
    const float **input_data,
    float *output_data,
    uint32_t input_count,
    uint32_t nsamples
) {
    for (uint32_t i = 0; i < nsamples; ++i) {
        output_data[i] = ((_i / (_samples_per_cycle / 2)) % 2 ? 1.0f : -1.0f) * _amplitude;
        _i = (_i + 1) % _samples_per_cycle;
    }
    return nsamples;
}

void SquareSampler::set_amplitude(float amplitude) {
    _amplitude = amplitude;
}
