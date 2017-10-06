#pragma once
#include <soundstone/Sampler.hpp>

namespace soundstone_test {
    class DumbSampler : public soundstone::Sampler {
    public:
        void setup(unsigned int sample_rate) override;
        void commit() override;
        size_t sample(float *data, size_t nsamples) override;
    };
}
