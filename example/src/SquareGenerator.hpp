#pragma once
#include <soundstone/Module.hpp>
#include <cstdint>

namespace soundstone_example {

    class SquareGenerator : public soundstone::Module {

        float _frequency = 0;
        float _amplitude = 0.6;
        uint32_t _samples_per_cycle = 0;
        uint32_t _i = 0;

        //
        // Module functions
        //

        void commit() override;
        void sample(const float * const *input_buffers, float *output_data, uint32_t nsamples) override;

    public:

        SquareGenerator(float frequency, uint32_t sample_rate);

        void set_amplitude(float amplitude);

    };
}
