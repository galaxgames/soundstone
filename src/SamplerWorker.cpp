#include <soundstone/SamplerWorker.hpp>
#include <cassert>
#include <algorithm>
#include <mutex>

using namespace soundstone;
using namespace std;

SamplerWorker::SamplerWorker()
    : _lock(_mutex)
{
    _is_running = true;
    _sampler_count = 0;
}

void SamplerWorker::setup(
    float **buffers, Sampler **samplers, size_t buffer_length,
    size_t sampler_count, Semaphore *semaphore
) {
    unique_lock<mutex> lock(_mutex);
    _buffer_length = buffer_length;

    if (_sampler_count != sampler_count) {
        _buffers = unique_ptr<float *[]>(new float *[sampler_count]);
        _samplers = unique_ptr<Sampler *[]>(new Sampler *[sampler_count]);
        //copy_n(buffers, _sampler_count, _buffers.get());
        //copy_n(samplers, _sampler_count, _samplers.get());
        for (size_t i = 0; i < sampler_count; ++i) {
            _buffers[i] = buffers[i];
            _samplers[i] = samplers[i];
        }

        _sampler_count = sampler_count;
    }

    _semaphore = semaphore;
    _is_waiting = false;
    lock.unlock();
    _cv.notify_all();
}

void SamplerWorker::process() {
    while (_is_running) {
        _is_waiting = true;
        while (_is_waiting) {
            _cv.wait(_lock);
        }
        if (_is_running) {
            // Generate sample data
            for (size_t i = 0; i < _sampler_count; ++i) {
                _samplers[i]->sample(_buffers[i], _buffer_length);
            }

            // Mix data together
            float *mix = _buffers[0];
            for (size_t i = 1; i < _sampler_count; ++i) {
                float *source = _buffers[i];
                for (size_t sample_index = 0; sample_index < _buffer_length; ++sample_index) {
                    float accumulated_sample = mix[sample_index];
                    mix[sample_index] = accumulated_sample + source[sample_index];
                }
            }

            _semaphore->return_worker(this);
        }
    }
}

void SamplerWorker::stop() {
    _is_running = false;
}

float *SamplerWorker::mixed_buffer() const {
    return _buffers[0];
}

Semaphore::Semaphore() {}
Semaphore::~Semaphore() {}

Semaphore::Semaphore(Semaphore &&other) noexcept {
    lock_guard<mutex> lock(_mutex);
    _free_workers = move(other._free_workers);
}

Semaphore &Semaphore::operator=(Semaphore &&other) noexcept {
    if (&other != this) {
        unique_lock<mutex> lhs_lock(_mutex, defer_lock);
        unique_lock<mutex> rhs_lock(other._mutex, defer_lock);
        lock(lhs_lock, rhs_lock);
        _free_workers = move(other._free_workers);
    }
    return *this;
}


void Semaphore::return_worker(SamplerWorker *worker) {
    unique_lock<mutex> lock(_mutex);
    //cout << "Returning worker" << endl;
    _free_workers.push_back(worker);
    _cv.notify_all();
}

SamplerWorker *Semaphore::wait() {
    unique_lock<mutex> lock(_mutex);
    while (_free_workers.size() < 1) {
        _cv.wait(lock);
    }
    SamplerWorker *worker = _free_workers.back();
    _free_workers.pop_back();
    return worker;
}
