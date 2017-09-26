#pragma once
#include <cstddef>

namespace soundstone {
    class Sampler {
    public:
        virtual ~Sampler() = default;
        virtual void setup(unsigned int sample_rate) = 0;
        virtual size_t sample(float *data, size_t nsamples) = 0;
    };

}
