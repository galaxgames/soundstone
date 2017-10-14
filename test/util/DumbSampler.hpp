#pragma once
#include <soundstone/Sampler.hpp>

namespace soundstone_test {
    class DumbSampler : public soundstone::Sampler {
    public:
        void setup(unsigned int sample_rate) override;
        void commit() override;
        size_t sample(
            const float **input_data,
            float *output_data,
            size_t input_count,
            size_t nsamples
        ) override;
    };
}
