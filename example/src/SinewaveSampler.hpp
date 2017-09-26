#include <soundstone/Sampler.hpp>

namespace soundstone_example {
    class SinewaveSampler : public soundstone::Sampler {
        float _t;
        float _frequency;
        unsigned int _sample_rate;

    public:
        SinewaveSampler(float frequency);
        void setup(unsigned int sample_rate) override;
        size_t sample(float *data, size_t nsamples) override;
    };
}
