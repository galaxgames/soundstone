#include "SquareSampler.hpp"

using namespace soundstone;
using namespace soundstone_example;
using namespace std;

SquareSampler::SquareSampler(float frequency) {
    _i = 0;
    _frequency = frequency;
    _amplitude = 1;
}

void SquareSampler::setup(unsigned int sample_rate) {
    _samples_per_cycle = sample_rate / static_cast<unsigned int>(_frequency);
}

void SquareSampler::commit() {

}

size_t SquareSampler::sample(
    const float **input_data,
    float *output_data,
    size_t input_count,
    size_t nsamples
) {
    for (size_t i = 0; i < nsamples; ++i) {
        output_data[i] = ((_i / (_samples_per_cycle / 2)) % 2 ? 1.0f : -1.0f) * _amplitude;
        _i = (_i + 1) % _samples_per_cycle;
    }
    return nsamples;
}

void SquareSampler::set_amplitude(float amplitude) {
    _amplitude = amplitude;
}
