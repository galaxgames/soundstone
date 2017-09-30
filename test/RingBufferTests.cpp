#include <gtest/gtest.h>
#include <soundstone/RingBuffer.hpp>
#include <array>

using namespace soundstone;
using namespace std;

TEST(RingBufferTests, TestProduceExpands) {
    RingBuffer<float> ringbuffer;
    array<float, 6> values = {{1, 2, 3, 4, 5, 6}};
    ringbuffer.produce(values.data(), 3);
    ringbuffer.produce(values.data(), 6);
    ASSERT_EQ(ringbuffer.size(), 9);
    ASSERT_EQ(ringbuffer.capacity(), 9);
}

TEST(RingBufferTests, TestProducingCorrectly) {
    RingBuffer<float> ringbuffer;
    array<float, 6> in_values = {{1, 2, 3, 4, 5, 6}};
    array<float, 9> out_values;
    array<float, 9> expected = {{1, 2, 3, 1, 2, 3, 4, 5, 6}};
    ringbuffer.produce(in_values.data(), 3);
    ringbuffer.produce(in_values.data(), 6);
    ringbuffer.consume(out_values.data(), 9);
    ASSERT_EQ(out_values, expected);
}

TEST(RingBufferTests, TestProducePastBoundary) {
    RingBuffer<float> ringbuffer;
    ringbuffer.reserve(6);
    array<float, 6> in_values = {{1, 2, 3, 4, 5, 6}};
    array<float, 4> out_values1;
    array<float, 6> out_values2;
    array<float, 4> expected_out1 = {{1, 2, 3, 4}};
    array<float, 6> expected_out2 = {{5, 6, 1, 2, 3, 4}};
    ringbuffer.produce(in_values.data(), 6);
    ringbuffer.consume(out_values1.data(), 4);
    ringbuffer.produce(in_values.data(),4);
    ringbuffer.consume(out_values2.data(), 6);
    ASSERT_EQ(out_values1, expected_out1);
    ASSERT_EQ(out_values2, expected_out2);
    ASSERT_EQ(ringbuffer.size(), 0);
}

TEST(RingBufferTests, TestProduceResize) {
    RingBuffer<float> ringbuffer;
    ringbuffer.reserve(3);
    array<float, 4> in_values = {{1, 2, 3, 4}};
    array<float, 4> expected_values = {{1, 2, 3, 4}};
    array<float, 4> out_values;
    ringbuffer.produce(in_values.data(), 2);
    ringbuffer.consume(out_values.data(), 2);
    ringbuffer.produce(in_values.data(), 4);
    ringbuffer.consume(out_values.data(), 4);
    ASSERT_EQ(expected_values, out_values);
}

TEST(RingBufferTests, TestResizeFirstCopyLengthIsCorrect) {
    // Make sure nothing bad happens if the length to the end of the buffer is
    // greater than the current size;
    RingBuffer<float> ringbuffer;
    ringbuffer.reserve(8);
    array<float, 8> in_values = {{1, 2, 3, 4, 5, 6, 7, 8}};
    array<float, 10> expected_values = {{1, 2, 1, 2, 3, 4, 5, 6, 7, 8}};
    array<float, 10> out_values;
    ringbuffer.produce(in_values.data(), 2);
    ringbuffer.produce(in_values.data(), 8);
    ringbuffer.consume(out_values.data(), 10);
    ASSERT_EQ(expected_values, out_values);
}

TEST(RingBufferTests, TestResizeFirstCopyLengthIsCorrectWithWrap) {
    // Make sure nothing bad happens if the length to the end of the buffer is
    // greater than the current size and the data is wrapping;
    RingBuffer<float> ringbuffer;
    ringbuffer.reserve(8);
    array<float, 12> in_values = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}};
    array<float, 5> expected_values = {{5, 6, 7, 8, 9}};
    array<float, 5> out_values;
    ringbuffer.produce(in_values.data(), 4);
    ringbuffer.consume(out_values.data(), 3);
    ringbuffer.produce(in_values.data() + 4, 3);
    ringbuffer.consume(out_values.data(), 1);
    // State at this point: capacity = 4, size = 3, offset = 0
    ringbuffer.produce(in_values.data() + 7, 2); // <- resize
    ringbuffer.consume(out_values.data(), 5);
    ASSERT_EQ(expected_values, out_values);
}
