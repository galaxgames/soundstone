#pragma once
#include <soundstone/Sampler.hpp>
#include <gmock/gmock.h>

namespace soundstone_test {
    class MockSampler : public soundstone::Sampler {
    public:
        MOCK_METHOD1(setup, void(unsigned int sample_rate));
        MOCK_METHOD0(commit, void());
        MOCK_METHOD4(sample, size_t(
            const float **input_data,
            float *output_data,
            size_t input_count,
            size_t nsamples
        ));
    };
}