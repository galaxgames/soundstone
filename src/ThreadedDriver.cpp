#include <soundstone/ThreadedDriver.hpp>
#include <iostream>

using namespace soundstone;
using namespace std;
using namespace std::chrono;

ThreadedDriver::ThreadedDriver(AudioProcessor *processor, const SystemAudio *system)
    : _processor(processor)
    , _system(system)
{
}

ThreadedDriver::~ThreadedDriver() {
    finish();
}

void ThreadedDriver::set_update_interval(std::chrono::nanoseconds ns) {
    _update_interval = ns;
}

void ThreadedDriver::set_latency_samples(uint32_t latency_samples) {
    _latency_samples = latency_samples;
}

void ThreadedDriver::start() {
    finish();

    _is_running = true;
    _thread = thread(bind(&ThreadedDriver::worker, this));
}

void ThreadedDriver::finish() {
    { lock_guard<mutex> lock(_is_running_mutex);
        _is_running = false;
        _is_running_condition.notify_all();
    }

    if (!_thread.joinable()) {
        return;
    }

    _thread.join();
}


void ThreadedDriver::worker() {
    uint32_t sample_rate = _system->sample_rate();

    double floating_interval_seconds = duration_cast<duration<double>>(_update_interval).count();
    uint32_t samples_per_frame = static_cast<uint32_t>(sample_rate * floating_interval_seconds);

    while (worker_should_run()) {
        auto start_time = steady_clock::now();

        uint32_t needed_samples = samples_per_frame + _latency_samples;
        uint32_t buffered_samples = _system->samples_buffered();

        if (needed_samples > buffered_samples) {
            _processor->update(needed_samples - buffered_samples);
        }

        this_thread::sleep_until(start_time + _update_interval);
    }
}

bool ThreadedDriver::worker_should_run() {
    unique_lock<mutex> lock(_is_running_mutex);
    return _is_running;
}
