#include "SquareGenerator.hpp"

using namespace soundstone;
using namespace soundstone_example;
using namespace std;

SquareGenerator::SquareGenerator(float frequency, uint32_t sample_rate) {
    _frequency = frequency;
    _samples_per_cycle = static_cast<uint32_t>(sample_rate / _frequency);
}

void SquareGenerator::commit() {

}

void SquareGenerator::sample(
    const float * const *input_data,
    float *output_data,
    uint32_t nsamples
) {
    for (uint32_t i = 0; i < nsamples; ++i) {
        output_data[i] = ((_i / (_samples_per_cycle / 2)) % 2 ? 1.0f : -1.0f) * _amplitude;
        _i = (_i + 1) % _samples_per_cycle;
    }
}

void SquareGenerator::set_amplitude(float amplitude) {
    _amplitude = amplitude;
}
