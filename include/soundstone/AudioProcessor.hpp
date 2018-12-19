#pragma once
#include "Sampler.hpp"
#include "SamplerWorker.hpp"
#include "DependencyGraph.hpp"
#include <soundstone/RingBuffer.hpp>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <soundstone/export.h>

namespace soundstone {

    class SOUNDSTONE_EXPORT AudioProcessor {
        uint32_t _sampler_buffer_length = 0;
        std::vector<Sampler *> _samplers;
        std::vector<std::unique_ptr<SamplerWorker>> _workers;
        std::vector<std::thread> _threads;
        std::vector<std::unique_ptr<float[]>> _sampler_buffers;
        Semaphore _semaphore;
        DependencyGraph<Sampler> _sampler_graph;
        std::vector<GraphNode<Sampler> *> _ordered_samplers;
        bool _is_graph_dirty = true;

        void setup_workers(uint32_t count);
        void mix(float *a, const float *b, uint32_t size);

    public:
        AudioProcessor();
        AudioProcessor(const AudioProcessor &) = delete;
        AudioProcessor(AudioProcessor &&) noexcept = default;
        ~AudioProcessor();
        AudioProcessor &operator=(const AudioProcessor &) = delete;
        AudioProcessor &operator=(AudioProcessor &&) noexcept = default;

        void add_sampler(Sampler *sampler);
        void route_sampler(Sampler *sampler, Sampler *target);
        void route_sampler_to_root(Sampler *sampler);
        void remove_sampler(Sampler *sampler);
        void update(float *data, uint32_t nsamples);
        void set_thread_count(uint32_t count);
    };
}
