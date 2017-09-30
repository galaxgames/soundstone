#pragma once
#include <cstddef>
#include <soundstone/testable_export.h>

namespace soundstone {
    template <typename T>
    class SOUNDSTONE_TESTABLE_EXPORT RingBuffer {
        size_t _capacity;
        size_t _size;
        size_t _start_offset;
        T *_buffer;


    public:
        RingBuffer();
        RingBuffer(const RingBuffer &);
        RingBuffer(RingBuffer &&) noexcept;
        RingBuffer &operator=(const RingBuffer &);
        RingBuffer &operator=(RingBuffer &&) noexcept;
        ~RingBuffer();

        size_t size() const;
        size_t capacity() const;
        void reserve(size_t capacity);
        void produce(const T *data, size_t count);
        void consume(T *data, size_t count);
    };

    extern template class RingBuffer<float>;
}

