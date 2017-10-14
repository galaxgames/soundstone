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
    _input_count = 0;
    _sampler_node = nullptr;
}

void SamplerWorker::reset() {
    _sampler_node = nullptr;
}

void SamplerWorker::setup(
    const float **input_buffers,
    float *output_buffer,
    const GraphNode<Sampler> *sampler_node,
    size_t input_count,
    size_t buffer_length,
    Semaphore *semaphore
) {
    unique_lock<mutex> lock(_mutex);
    _buffer_length = buffer_length;

    if (_input_count != input_count) {
        _input_buffers = unique_ptr<const float *[]>(new const float *[input_count]);
        for (size_t i = 0; i < input_count; ++i) {
            _input_buffers[i] = input_buffers[i];
        }
        _input_count = input_count;
    }

    _sampler_node = sampler_node;
    _output_buffer = output_buffer;
    _buffer_length = buffer_length;
    _semaphore = semaphore;
    _is_waiting = false;
    _cv.notify_all();
}

void SamplerWorker::process() {
    while (_is_running) {
        _is_waiting = true;
        while (_is_waiting) {
            _cv.wait(_lock);
        }
        if (_is_running) {
            _sampler_node->data->sample(_input_buffers.get(), _output_buffer, _input_count, _buffer_length);
            _semaphore->return_worker(this);
        }
    }
}

void SamplerWorker::stop() {
    unique_lock<mutex> lock(_mutex);
    _is_waiting = false;
    _is_running = false;
    _cv.notify_all();
}

const GraphNode<Sampler> *SamplerWorker::sampler_node() const {
    return _sampler_node;
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
    while (_free_workers.empty()) {
        _cv.wait(lock);
    }
    SamplerWorker *worker = _free_workers.back();
    _free_workers.pop_back();
    return worker;
}
