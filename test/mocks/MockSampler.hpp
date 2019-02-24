#pragma once
#include <soundstone/Module.hpp>
#include <gmock/gmock.h>

namespace soundstone_test {
    class MockSampler : public soundstone::Module {
    public:
        MOCK_METHOD0(commit, void());
        MOCK_METHOD3(sample, void(
            const float * const *input_buffers,
            float *output_buffer,
            uint32_t nsamples
        ));
    };
}