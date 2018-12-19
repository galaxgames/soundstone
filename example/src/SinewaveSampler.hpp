#include <soundstone/Sampler.hpp>
#include <memory>

namespace soundstone_example {
    class SinewaveSampler : public soundstone::Sampler {
        uint32_t _frequency;
        uint32_t _samples_per_cycle;
        uint32_t _i;
        std::unique_ptr<float[]> _samples;

    public:
        SinewaveSampler(uint32_t frequency, uint32_t sample_rate);
        void commit() override;
        uint32_t sample(
            const float **input_data,
            float *output_data,
            uint32_t input_count,
            uint32_t nsamples
        ) override;
    };
}
