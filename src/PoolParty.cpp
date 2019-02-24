#include <soundstone/PoolParty.hpp>

using namespace soundstone;
using namespace std;
using namespace placeholders;


PoolParty::~PoolParty() {
    shutdown();
}

void PoolParty::shutdown() {
    // Signal all conditions to ensure all the threads can exit.
    { lock_guard<mutex> lock(_start_mutex);
        _should_quit = true;
        _start_condition.notify_all();
    }

    // Join all threads
    for (uint32_t i = 0; i < _worker_count; ++i) {
        _threads[i].join();
    }
}

void PoolParty::setup(uint32_t worker_count) {
    shutdown();
    _should_quit = false;

    _worker_count = worker_count;
    _threads = unique_ptr<thread[]>(new thread[worker_count]);

    for (uint32_t i = 0; i < worker_count; ++i) {
        _threads[i] = thread(&PoolParty::worker_routine, this);
    }
}

void PoolParty::add_work(std::function<void()> function) {
    add_work(move(function), nullptr, 0);
}

void PoolParty::add_work(std::function<void()> function, const uint32_t *dependencies, uint32_t dependency_count) {
    WorkInfo info;
    info.id = _work.size();
    info.func = move(function);
    info.dependencies = dependencies;
    info.dependency_count = dependency_count;
    _work.emplace_back(move(info));
}

void PoolParty::work() {
    // Note: If the rules are followed (work is not called while in progress), then all of the threads are waiting at
    // the start condition.

    // Initialize completion statuses (all to false)
    _completion_status = unique_ptr<bool[]>(new bool[_work.size()]());

    // Keep the finish mutex locked from this point forward while starting worker threads until we are able to
    // wait on them.
    unique_lock<mutex> finish_lock(_finished_mutex);

    // Signal all workers to start
    { lock_guard<mutex> lock(_start_mutex);
        _is_working = true;
        _start_condition.notify_all();
    }

    // Wait for all workers to finish
    for (uint32_t i = 0; i < _worker_count; ++i) {
        while (!_work.empty()) {
            _finished_condition.wait(finish_lock);
        }
    }

    _work.clear();
}

void PoolParty::worker_routine() {
    WorkInfo work = {};

    while (true) {

        // Check if the workers should rise up and go on strike.
        { unique_lock<mutex> lock(_start_mutex);
            while (!(_should_quit || _is_working)) {
                _start_condition.wait(lock);
            }

            if (_should_quit) {
                break;
            }
        }

        // We've got work to do.
        bool has_work = true;

        while (true) {

            // Figure out what to do next
            bool found_work = false;

            while (true) {
                unique_lock<mutex> lock(_work_mutex);

                if (_work.empty()) {
                    // No more work to do.
                    has_work = false;
                    break;
                }

                // Figure out what to work on next.
                // Reverse iterate through to work list so that it is efficient to delete items, given that the work is
                // given to the pool party in reverse dependency order.
                for (size_t i = _work.size(); i > 0; --i) {
                    WorkInfo &info = _work[i - 1];
                    found_work = true;

                    for (uint32_t j = 0; j < info.dependency_count; ++j) {
                        uint32_t dependency_index = info.dependencies[j];
                        if (!_completion_status[dependency_index]) {
                            found_work = false;
                            break;
                        }
                    }

                    if (found_work) {
                        work = info;
                        _work.erase(_work.begin() + i - 1);
                        break;
                    }
                }

                if (!found_work) {
                    // No work that we're allowed to work on. Wait until some work gets done and try again.
                    _update_condition.wait(lock);
                } else {
                    // Work was found, so let's break outta here and do the work!
                    break;
                }
            }

            if (!has_work) {
                // There's no work to do, so let's get outta here.
                break;
            }

            work.func();

            // Notify the other friends which may be waiting due to work dependencies.
            { lock_guard<mutex> lock(_work_mutex);
                // While we have the work mutex locked, we also need to set the completion status of this work.
                _completion_status[work.id] = true;

                _update_condition.notify_all();
            }
        }

        // Tell the work invoking thread that the work is done
        { lock_guard<mutex> lock(_finished_mutex);
            _is_working = false;
            _finished_condition.notify_all();

        }

        // Back to top
    }

    // Worker has quit.
}

