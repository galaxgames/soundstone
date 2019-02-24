#pragma once
#include <soundstone/Module.hpp>

namespace soundstone_test {
    class DumbSampler : public soundstone::Module {
    public:
        void commit() override;
        void sample(
            const float * const *input_data,
            float *output_data,
            uint32_t nsamples
        ) override;
    };
}
