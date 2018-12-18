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
        size_t _sampler_buffer_length;
        std::vector<Sampler *> _samplers;
        std::vector<std::unique_ptr<SamplerWorker>> _workers;
        std::vector<std::thread> _threads;
        std::vector<std::unique_ptr<float[]>> _sampler_buffers;
        Semaphore _semaphore;
        DependencyGraph<Sampler> _sampler_graph;
        std::vector<GraphNode<Sampler> *> _ordered_samplers;
        bool _is_graph_dirty;
        unsigned int _sample_rate;

        void move_internal(AudioProcessor &&other) noexcept;
        void setup_workers(size_t count);
        void mix(float *a, const float *b, size_t size);

    public:
        AudioProcessor();
        AudioProcessor(const AudioProcessor &) = delete;
        AudioProcessor(AudioProcessor &&) noexcept;
        ~AudioProcessor();
        AudioProcessor &operator=(const AudioProcessor &) = delete;
        AudioProcessor &operator=(AudioProcessor &&) noexcept;

        void add_sampler(Sampler *sampler);
        void route_sampler(Sampler *sampler, Sampler *target);
        void route_sampler_to_root(Sampler *sampler);
        void remove_sampler(Sampler *sampler);
        void update(float *data, size_t nsamples);
        void set_sample_rate(unsigned int sample_rate);
        void set_thread_count(size_t count);
    };
}
