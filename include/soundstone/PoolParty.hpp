#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>

namespace soundstone {

    class PoolParty final {
        class WorkInfo {
        public:
            std::function<void()> func;
            uint32_t id;
            const uint32_t *dependencies;
            uint32_t dependency_count;
        };

        std::unique_ptr<std::thread[]> _threads;

        std::vector<WorkInfo> _work;
        std::unique_ptr<bool[]> _completion_status;
        std::mutex _work_mutex;
        std::condition_variable _update_condition;

        std::condition_variable _start_condition;
        std::mutex _start_mutex;

        std::condition_variable _finished_condition;
        std::mutex _finished_mutex;

        uint32_t _worker_count = 0;

        bool _is_working = false;
        //std::mutex _is_working_mutex;

        bool _should_quit = false;
        //std::mutex _should_quit_mutex;

        void worker_routine();
        void shutdown();

    public:
        ~PoolParty();
        void setup(uint32_t worker_count);
        void add_work(std::function<void()> function);
        void add_work(std::function<void()> function, const uint32_t *dependency, uint32_t dependency_count);
        void work();
    };

}

