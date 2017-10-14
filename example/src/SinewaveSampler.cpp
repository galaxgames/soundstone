#include "SinewaveSampler.hpp"
#include <cmath>

#include <iostream>

using namespace soundstone_example;
using namespace soundstone;
using namespace std;

SinewaveSampler::SinewaveSampler(unsigned int frequency) {
    _frequency = frequency;
    _i = 0;
}

void SinewaveSampler::setup(unsigned int sample_rate) {
    _sample_rate = sample_rate;
    _samples_per_cycle = sample_rate / _frequency;
    _samples = unique_ptr<float[]>(new float[_samples_per_cycle]);
    for (size_t i = 0; i < _samples_per_cycle; ++i) {
        float t = (float)i / (float)_samples_per_cycle;
        _samples[i] = sin(t * 3.14159265359f * 2.0f);
    }
}

void SinewaveSampler::commit() {

}

size_t SinewaveSampler::sample(
    const float **input_data,
    float *output_data,
    size_t input_count,
    size_t nsamples
) {
    for (size_t i = 0; i < nsamples; ++i) {
        output_data[i] = _samples[_i];
        _i = (_i + 1) % _samples_per_cycle;
    }
    return nsamples;
}
