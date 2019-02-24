#pragma once
#include <soundstone/Module.hpp>

namespace soundstone_example {

    class Mixer : public soundstone::Module {

        void commit() override;
        void sample(const float *const *input_buffers, float *output_buffer, uint32_t nsamples) override;

    public:

    };


}

