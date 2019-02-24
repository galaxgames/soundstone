#include <soundstone/AudioProcessor.hpp>

#include "mocks/MockSampler.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

using namespace soundstone;
using namespace soundstone_test;
using namespace testing;
using namespace std;

class AudioProcessorTests : public TestWithParam<uint32_t> {

};


TEST(AudioProcessorTests, TestConstructDestruct)
{
    AudioProcessor processor;
}

TEST_P(AudioProcessorTests, TestUpdateWithZeroSamplers)
{
    AudioProcessor processor;
    processor.set_thread_count(GetParam());
    processor.update(1024);
}

TEST_P(AudioProcessorTests, TestRemovedSamplerAreNotCalled)
{
    NiceMock<MockSampler> sampler1, sampler2;
    AudioProcessor processor;
    processor.set_thread_count(GetParam());

    EXPECT_CALL(sampler1, sample(NotNull(), NotNull(), 1)).Times(1);
    EXPECT_CALL(sampler2, sample(NotNull(), NotNull(), _)).Times(0);

    processor.add_module(&sampler1);
    processor.add_module(&sampler2);
    processor.set_input(&sampler1, 0, &sampler2);
    processor.remove_module(&sampler2);
    processor.update(1);
}

TEST_P(AudioProcessorTests, TestSimpleRoutingWorks)
{
    // Module 1 -> Module 2
    NiceMock<MockSampler> sampler1, sampler2;
    AudioProcessor processor;
    processor.set_thread_count(GetParam());

    EXPECT_CALL(sampler1, sample(Pointee(NotNull()), NotNull(), 1)).WillOnce(DoAll(SetArgPointee<1>(1.0f)));
    EXPECT_CALL(sampler2, sample(Pointee(Pointee(1.0f)), NotNull(), 1)).Times(1);

    processor.add_module(&sampler1);
    processor.add_module(&sampler2);
    processor.set_input(&sampler2, 0, &sampler1);
    processor.update(1);
}

MATCHER_P2(PointeeAtIndex, n, m, "") {
    return Matches(m)(arg[n]);
}

MATCHER_P3(AnyPointeeAtIndices, from, to, m, "") {
    for (size_t i = from; i < to; ++i) {
        if (Matches(m)(arg[i])) {
            return true;
        }
    }
    return false;
}

MATCHER_P3(AllPointeesAtIndices, from, to, m, "") {
    for (size_t i = from; i < to; ++i) {
        if (!Matches(m)(arg[i])) {
            return false;
        }
    }
    return true;
}

TEST_P(AudioProcessorTests, TestRoutingMultipleInputsToSamplerWithDifferentInputIndices)
{
    // Module 1 - Module 3
    //           /
    // Module 2
    NiceMock<MockSampler> sampler1, sampler2, sampler3;
    AudioProcessor processor;
    processor.set_thread_count(GetParam());

    EXPECT_CALL(sampler1, sample(_, NotNull(), 1)).WillOnce(DoAll(SetArgPointee<1>(1.0f)));
    EXPECT_CALL(sampler2, sample(_, NotNull(), 1)).WillOnce(DoAll(SetArgPointee<1>(2.0f)));
    EXPECT_CALL(sampler3, sample(
        AllOf(PointeeAtIndex(0, Pointee(1.0f)), PointeeAtIndex(1, Pointee(2.0f))), NotNull(), 1
    )).Times(1);

    processor.add_module(&sampler1);
    processor.add_module(&sampler2);
    processor.add_module(&sampler3);
    processor.set_input(&sampler3, 0, &sampler1);
    processor.set_input(&sampler3, 1, &sampler2);
    processor.update(1);
}

TEST_P(AudioProcessorTests, TestRoutingDiamondWorks)
{
    //             Module 2
    // Module 1 <           > root
    //             Module 3
    NiceMock<MockSampler> sampler1, sampler2, sampler3, sampler4;
    AudioProcessor processor;
    processor.set_thread_count(GetParam());
    auto buff = unique_ptr<float[]>(new float[1]);

    EXPECT_CALL(sampler1, sample(
        NotNull(), NotNull(), 1
    )).Times(1).WillOnce(DoAll(SetArgPointee<1>(1.0f)));

    EXPECT_CALL(sampler2, sample(
        PointeeAtIndex(0, Pointee(1.0f)), NotNull(), 1
    )).Times(1).WillOnce(DoAll(SetArgPointee<1>(2.0f)));

    EXPECT_CALL(sampler3, sample(
        PointeeAtIndex(0, Pointee(1.0f)), NotNull(), 1
    )).Times(1).WillOnce(DoAll(SetArgPointee<1>(3.0f)));

    EXPECT_CALL(sampler4, sample(
        AllOf(PointeeAtIndex(0, Pointee(2.0f)), PointeeAtIndex(1, Pointee(3.0))),
        NotNull(),
        1
    )).Times(1);

    processor.add_module(&sampler1);
    processor.add_module(&sampler2);
    processor.add_module(&sampler3);
    processor.add_module(&sampler4);
    processor.set_input(&sampler4, 0, &sampler2);
    processor.set_input(&sampler4, 1, &sampler3);
    processor.set_input(&sampler2, 0, &sampler1);
    processor.set_input(&sampler3, 0, &sampler1);
    processor.update(1);
}


INSTANTIATE_TEST_SUITE_P(
    AudioProcessorTestsImpl,
    AudioProcessorTests,
    ::testing::Values(1, 2, 3, 4));
