#pragma once
#include <soundstone/Sampler.hpp>

namespace soundstone_example {
    class SquareSampler : public soundstone::Sampler {
        float _frequency;
        float _amplitude;
        unsigned int _samples_per_cycle;
        unsigned int _i;
    public:
        SquareSampler(float frequency);
        void setup(unsigned int sample_rate) override;
        size_t sample(float *data, size_t nsamples) override;
        void set_amplitude(float amplitude);
    };
}
