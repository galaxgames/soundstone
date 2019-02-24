#include <soundstone/Module.hpp>
#include <memory>

namespace soundstone_example {
    class SinewaveGenerator : public soundstone::Module {
        uint32_t _frequency = 0;
        uint32_t _samples_per_cycle = 0;
        uint32_t _i = 0;
        float _amplitude = 0.6;
        std::unique_ptr<float[]> _samples;

        //
        // Module methods
        //

        void commit() override;
        void sample(const float * const *input_data, float *output_data, uint32_t nsamples) override;

    public:

        SinewaveGenerator(uint32_t frequency, uint32_t sample_rate);

        void set_amplitude(float amplitude);

    };
}
