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
        explicit SinewaveSampler(unsigned int frequency);
        void setup(unsigned int sample_rate) override;
        void commit() override;
        size_t sample(
            const float **input_data,
            float *output_data,
            size_t input_count,
            size_t nsamples
        ) override;
    };
}
