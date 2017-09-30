#include <soundstone/Sampler.hpp>
#include <memory>

namespace soundstone_example {
    class SinewaveSampler : public soundstone::Sampler {
        unsigned int _frequency;
        unsigned int _sample_rate;
        unsigned int _samples_per_cycle;
        size_t _i;
        std::unique_ptr<float[]> _samples;

    public:
        SinewaveSampler(unsigned int frequency);
        void setup(unsigned int sample_rate) override;
        size_t sample(float *data, size_t nsamples) override;
    };
}
