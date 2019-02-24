#include "Mixer.hpp"

using namespace soundstone_example;
using namespace soundstone;

void Mixer::commit() {

}

void Mixer::sample(const float *const *input_buffers, float *output_buffer, uint32_t nsamples) {
    for (uint32_t i = 0; i < nsamples; ++i) {
        output_buffer[i] = (input_buffers[0][i] + input_buffers[1][i]);
    }
}
