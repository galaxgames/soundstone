#pragma once
#include <soundstone/Sampler.hpp>

namespace soundstone_example {
    class SquareSampler : public soundstone::Sampler {
        float _frequency;
        float _amplitude;
        unsigned int _samples_per_cycle;
        unsigned int _i;
    public:
        explicit SquareSampler(float frequency);
        void setup(unsigned int sample_rate) override;
        void commit() override;
        size_t sample(
            const float **input_data,
            float *output_data,
            size_t input_count,
            size_t nsamples
        ) override;
        void set_amplitude(float amplitude);
    };
}
