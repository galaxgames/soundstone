#pragma once
#include <cstddef>
#include <cstdint>
#include <soundstone/export.h>

namespace soundstone {

    class SOUNDSTONE_EXPORT Sampler {
    public:
        virtual ~Sampler() = default;

        /**
         * @brief commit Called when sample is about to be called.
         *
         * Implementations of this funciton should copy state needed by the sample function to prevent race conditions.
         * No audio processing takes place until all samplers in the graph have had their commit method called.
         */
        virtual void commit() = 0;

        /**
         * @brief sample Generate samples
         * @param input_buffers Data from samplers routed to output to this sampler.
         * @param output_buffer Buffer to sample to.
         * @param input_count   The count of buffers the input_buffers parameter points to.
         * @param nsamples      Length of the given buffers.
         * @return              The count of samples actually generated.
         */
        virtual uint32_t sample(
            const float **input_buffers,
            float *output_buffer,
            uint32_t input_count,
            uint32_t nsamples
        ) = 0;
    };

}
