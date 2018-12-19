#include <soundstone/RingBuffer.hpp>
#include <utility>
#include <algorithm>
#include <cassert>

using namespace soundstone;
using namespace std;

template <typename T>
RingBuffer<T>::RingBuffer() = default;

template <typename T>
RingBuffer<T>::RingBuffer(const RingBuffer &other) {
    *this = other;
}

template <typename T>
RingBuffer<T>::RingBuffer(RingBuffer &&other) noexcept {
    *this = move(other);
}

template <typename T>
RingBuffer<T> &RingBuffer<T>::operator=(const RingBuffer &other) {
    _capacity = other._capacity;
    _size = other._size;
    _start_offset = other._start_offset;

    _buffer = new T[_size];
    copy_n(other._buffer, _capacity, _buffer);

    return *this;
}

template <typename T>
RingBuffer<T> &RingBuffer<T>::operator=(RingBuffer &&other) noexcept {
    _capacity = other._capacity;
    _size = other._size;
    _start_offset = other._size;
    _buffer = other._buffer;
    other._buffer = nullptr;
    return *this;
}

template <typename T>
RingBuffer<T>::~RingBuffer() {
    delete [] _buffer;
}

template <typename T>
size_t RingBuffer<T>::size() const {
    return _size;
}

template <typename T>
size_t RingBuffer<T>::capacity() const {
    return _capacity;
}

template <typename T>
void RingBuffer<T>::reserve(size_t capacity) {
    T *temp_buffer = new T[capacity];

    size_t right = min(_start_offset + _size, _capacity);
    size_t first_count = right - _start_offset;
    size_t second_count = _size - first_count;
    copy_n(_buffer + _start_offset, first_count, temp_buffer);
    copy_n(_buffer, second_count, temp_buffer + first_count);
    _capacity = capacity;
    delete [] _buffer;
    _buffer = temp_buffer;
    _start_offset = 0;
}

template <typename T>
void RingBuffer<T>::produce(const T *data, size_t count) {
    size_t new_size = _size + count;
    if (new_size > _capacity) {
        reserve(new_size);
    }

    size_t right = min(_start_offset + _size, _capacity);
    size_t first_count = min(_capacity - right, count);
    size_t left = _size - (right - _start_offset);
    size_t second_count = count - first_count;
    copy_n(data, first_count, _buffer + right);
    copy_n(data + first_count, second_count, _buffer + left);
    _size = new_size;
}

template <typename T>
void RingBuffer<T>::consume(T *data, size_t count) {
    if (count == 0) {
        return;
    }
    assert(count <= _size);
    size_t first_count = min(count, _capacity - _start_offset);
    size_t second_count = count - first_count;
    copy_n(_buffer + _start_offset, first_count, data);
    copy_n(_buffer, second_count, data + first_count);
    _start_offset = (_start_offset + count) % _capacity;
    _size -= count;
}

namespace soundstone {
    template class RingBuffer<float>;
}

