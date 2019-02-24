#pragma once
#include "AudioProcessor.hpp"
#include "SystemAudio.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace soundstone {
    class ThreadedDriver final {

        AudioProcessor *_processor = nullptr;
        const SystemAudio *_system = nullptr;

        std::thread _thread;

        bool _is_running = false;
        std::mutex _is_running_mutex;
        std::condition_variable _is_running_condition;

        std::chrono::nanoseconds _update_interval = std::chrono::milliseconds(15);
        uint32_t _latency_samples = 512;

        void worker();
        bool worker_should_run();

    public:
        ThreadedDriver(soundstone::AudioProcessor *processor, const SystemAudio *system);
        ~ThreadedDriver();

        void set_update_interval(std::chrono::nanoseconds ns);
        void set_latency_samples(uint32_t latency_samples);
        void start();
        void finish();

    };
}

