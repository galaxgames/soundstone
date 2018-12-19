#pragma once
#include <soundstone/Sampler.hpp>
#include <cstdint>

namespace soundstone_example {
    class SquareSampler : public soundstone::Sampler {
        float _frequency;
        float _amplitude;
        uint32_t _samples_per_cycle;
        uint32_t _i;
    public:
        SquareSampler(float frequency, uint32_t sample_rate);
        void commit() override;
        uint32_t sample(
            const float **input_data,
            float *output_data,
            uint32_t input_count,
            uint32_t nsamples
        ) override;
        void set_amplitude(float amplitude);
    };
}
