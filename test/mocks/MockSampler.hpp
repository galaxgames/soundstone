#pragma once
#include <soundstone/Sampler.hpp>
#include <gmock/gmock.h>

namespace soundstone_test {
    class MockSampler : public soundstone::Sampler {
    public:
        MOCK_METHOD1(setup, void(unsigned int sample_rate));
        MOCK_METHOD0(commit, void());
        MOCK_METHOD4(sample, uint32_t(
            const float **input_data,
            float *output_data,
            uint32_t input_count,
            uint32_t nsamples
        ));
    };
}