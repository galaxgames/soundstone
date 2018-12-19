#pragma once
#include <soundstone/Sampler.hpp>

namespace soundstone_test {
    class DumbSampler : public soundstone::Sampler {
    public:
        void commit() override;
        uint32_t sample(
            const float **input_data,
            float *output_data,
            uint32_t input_count,
            uint32_t nsamples
        ) override;
    };
}
