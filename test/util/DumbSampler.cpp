#include "DumbSampler.hpp"

using namespace soundstone_test;

void DumbSampler::setup(unsigned int sample_rate) {}
void DumbSampler::commit() {}
size_t DumbSampler::sample(
    const float **input_buffers,
    float *output_buffer,
    size_t input_count,
    size_t nsamples
) {
    return 0;
}
