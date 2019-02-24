#pragma once
#include <cstddef>
#include <cstdint>
#include <soundstone/export.h>

namespace soundstone {

    class SOUNDSTONE_EXPORT Module {
    public:
        virtual ~Module() = default;

        /**
         * @brief commit Called when sample is about to be called.
         *
         * Implementations of this funciton should copy state needed by the sample function to prevent race conditions.
         * No audio processing takes place until all samplers in the graph have had their commit method called.
         */
        virtual void commit() = 0;

        /**
         * @brief sample Generate samples
         * @param input_buffers Sample data from modules routed to this module.
         * @param output_buffer Buffer to sample into.
         * @param nsamples      Length of the given buffers.
         */
        virtual void sample(
            const float * const *input_buffers,
            float *output_buffer,
            uint32_t nsamples
        ) = 0;
    };

}
