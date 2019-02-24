#pragma once
#include "Module.hpp"
#include "SystemAudio.hpp"

namespace soundstone {
    class SystemOutputModule : public Module {
        SystemAudio *_audio = nullptr;

    public:
        SystemOutputModule(SystemAudio *audio);

        void commit() override;
        void sample(const float *const *input_buffers, float *output_buffer, uint32_t nsamples) override;
    };
}


