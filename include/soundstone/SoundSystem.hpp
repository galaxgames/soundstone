#pragma once
#include "Sampler.hpp"
#include "SamplerWorker.hpp"
#include "DependencyGraph.hpp"
#include <soundstone/RingBuffer.hpp>
#include <cubeb/cubeb.h>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>

namespace soundstone {


    class SoundSystem {

        // Moveable primitives
        cubeb *_cubeb;
        cubeb_stream *_stream;

        cubeb_state _state;
        unsigned int _sample_rate;
        unsigned int _latency;
        size_t _sampler_buffer_length;

        std::vector<Sampler *> _samplers;
        std::vector<std::unique_ptr<SamplerWorker>> _workers;
        std::vector<std::thread> _threads;
        std::vector<std::unique_ptr<float[]>> _sampler_buffers;
        Semaphore _semaphore;
        RingBuffer<float> _data;
        std::mutex _data_mutex;
        DependencyGraph<Sampler> _sampler_graph;
        std::vector<GraphNode<Sampler> *> _ordered_samplers;
        bool _is_graph_dirty;

        void move_internal(SoundSystem &&other) noexcept;
        bool init_cubeb();
        void destroy_cubeb();
        void setup_workers(size_t count);

        static long data_callback(
            cubeb_stream *stream, void *user_ptr, void const *input_buffer,
            void *output_buffer, long nframes
        );

        static void state_callback(
            cubeb_stream *stream, void *user_ptr, cubeb_state state
        );

        void mix(float *a, const float *b, size_t size);

    public:
        SoundSystem();
        SoundSystem(const SoundSystem &) = delete;
        SoundSystem(SoundSystem &&) noexcept;
        ~SoundSystem();
        SoundSystem &operator=(const SoundSystem &) = delete;
        SoundSystem &operator=(SoundSystem &&) noexcept;

        bool is_ok() const;
        bool is_steam_ok() const;
        bool is_stream_playing() const;
        bool is_stream_drained() const;
        size_t samples_buffered() const;
        unsigned int sample_rate() const;
        void add_sampler(Sampler *sampler);
        void route_sampler(Sampler *sampler, Sampler *target);
        void route_sampler_to_root(Sampler *sampler);
        void remove_sampler(Sampler *sampler);
        void update(size_t nsamples);
        void set_thread_count(size_t count);
    };
}
