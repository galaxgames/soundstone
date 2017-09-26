#include "SinewaveSampler.hpp"
#include <cmath>

using namespace soundstone_example;
using namespace soundstone;
using namespace std;

SinewaveSampler::SinewaveSampler(float frequency) {
    _t = 0;
    _frequency = frequency;
}

void SinewaveSampler::setup(unsigned int sample_rate) {
    _sample_rate = sample_rate;
}

size_t SinewaveSampler::sample(float *data, size_t nsamples) {
    float t_step = 1.0f / (float)_sample_rate;
    for (size_t i = 0; i < nsamples; ++i) {
        data[i] = sin(_t * _frequency);
        _t += t_step;
    }
    return nsamples;
}
