#include "DumbSampler.hpp"

using namespace soundstone_test;

void DumbSampler::commit() {}

uint32_t DumbSampler::sample(
    const float **input_buffers,
    float *output_buffer,
    uint32_t input_count,
    uint32_t nsamples
) {
    return 0;
}
