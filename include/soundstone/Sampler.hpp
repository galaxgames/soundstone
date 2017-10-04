#pragma once
#include <cstddef>

namespace soundstone {
    class Sampler {
    public:
        virtual ~Sampler() = default;

        /**
         * @brief setup       Called to set parameters from the sound system.
         * @param sample_rate The sample rate of the sound system using this
         *                    sampler.
         */
        virtual void setup(unsigned int sample_rate) = 0;

        /**
         * @brief commit Called when sample is about to be called.
         *
         * Implementations of this funciton should copy state needed by the
         * sample function to prevent race conditions.
         */
        virtual void commit() = 0;

        /**
         * @brief sample Generate samples
         * @param data     Buffer to place generated sample data into.
         * @param nsamples Length of the given buffer.
         * @return         The count of samples actually generated.
         */
        virtual size_t sample(float *data, size_t nsamples) = 0;
    };

}
