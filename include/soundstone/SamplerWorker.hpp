#pragma once
#include "Sampler.hpp"
#include <thread>
#include <cstddef>
#include <vector>

namespace soundstone {

    class SamplerWorker;

    class Semaphore {
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
        std::unique_ptr<Sampler *[]> _samplers;
        std::unique_ptr<float *[]> _buffers;
        Semaphore *_semaphore;
        size_t _buffer_length;
        size_t _sampler_count;
        bool _is_running;
        bool _is_waiting;
    public:
        SamplerWorker();
        void setup(
            float **buffers, Sampler **samplers, size_t buffer_length,
            size_t sampler_count, Semaphore *semaphore
        );
        void process();
        void stop();
        float *mixed_buffer() const;
    };

}

