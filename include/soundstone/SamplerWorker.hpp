#pragma once
#include "Module.hpp"
#include "DependencyGraph.hpp"
#include <cstddef>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <soundstone/export.h>

namespace soundstone {

    class SamplerWorker;

    class SOUNDSTONE_EXPORT Semaphore {
        std::vector<SamplerWorker *> _free_workers;
        std::condition_variable _cv;
        std::mutex _mutex;
    public:
        Semaphore();
        ~Semaphore();
        Semaphore(Semaphore &&) noexcept;
        Semaphore &operator=(Semaphore &&) noexcept;

        void return_worker(SamplerWorker *worker);
        SamplerWorker *wait();
    };


    class SamplerWorker {
        std::condition_variable _cv;
        std::mutex _mutex;
        std::unique_lock<std::mutex> _lock;
        const GraphNode<Module> *_sampler_node;
        std::unique_ptr<const float *[]> _input_buffers;
        float *_output_buffer;
        Semaphore *_semaphore;
        size_t _input_count;
        size_t _buffer_length;
        bool _is_running;
        bool _is_waiting;
    public:
        SamplerWorker();
        void reset();
        void setup(
            const float **input_buffers,
            float *output_buffer,
            const GraphNode<Module> *sampler_node,
            size_t input_count,
            size_t buffer_length,
            Semaphore *semaphore
        );
        void process();
        void stop();
        const GraphNode<Module> *sampler_node() const;
    };

}

