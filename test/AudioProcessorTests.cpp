#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <soundstone/AudioProcessor.hpp>
#include "mocks/MockSampler.hpp"
#include <memory>

using namespace soundstone;
using namespace soundstone_test;
using namespace testing;
using namespace std;

TEST(AudioProcessorTests, TestConstructDestruct)
{
    AudioProcessor processor;
}

TEST(AudioProcessorTests, TestUpdateWithZeroSamplers)
{
    AudioProcessor processor;
    auto buff = unique_ptr<float[]>(new float[1024]);
    processor.update(buff.get(), 1024);
}

TEST(AudioProcessorTests, TestRemoveSampler)
{
    NiceMock<MockSampler> sampler1, sampler2;
    AudioProcessor processor;
    auto buff = unique_ptr<float[]>(new float[1]);

    EXPECT_CALL(sampler1, sample(_, NotNull(), 0, 1)).Times(1);
    EXPECT_CALL(sampler2, sample(_, NotNull(), _, _)).Times(0);

    processor.add_sampler(&sampler1);
    processor.add_sampler(&sampler2);
    processor.route_sampler_to_root(&sampler1);
    processor.route_sampler_to_root(&sampler2);
    processor.remove_sampler(&sampler2);
    processor.update(buff.get(), 1);
}

TEST(AudioProcessorTests, TestSimpleRouting)
{
    // Sampler 1 -> Sampler 2 -> root
    NiceMock<MockSampler> sampler1, sampler2;
    AudioProcessor system;
    auto buff = unique_ptr<float[]>(new float[1]);

    EXPECT_CALL(sampler1, sample(_, NotNull(), 0, 1)).WillOnce(DoAll(SetArgPointee<1>(1.0f), Return(1)));
    EXPECT_CALL(sampler2, sample(Pointee(Pointee(1)), NotNull(), 1, 1)).Times(1);

    system.add_sampler(&sampler1);
    system.add_sampler(&sampler2);
    system.route_sampler(&sampler1, &sampler2);
    system.route_sampler_to_root(&sampler2);
    system.update(buff.get(), 1);
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

TEST(AudioProcessorTests, TestRoutingMultipleInputs)
{
    // Sampler 1 - Sampler 3 -> root
    //           /
    // Sampler 2
    NiceMock<MockSampler> sampler1, sampler2, sampler3;
    AudioProcessor system;
    auto buff = unique_ptr<float[]>(new float[1]);

    EXPECT_CALL(sampler1, sample(_, NotNull(), 0, 1)).WillOnce(DoAll(SetArgPointee<1>(1.0f), Return(1)));
    EXPECT_CALL(sampler2, sample(_, NotNull(), 0, 1)).WillOnce(DoAll(SetArgPointee<1>(2.0f), Return(1)));
    EXPECT_CALL(sampler3, sample(
        AnyPointeeAtIndices(0, 2, AnyOf(Pointee(1.0f), Pointee(2.0f))),
        NotNull(), 2, 1
    )).Times(1);

    system.add_sampler(&sampler1);
    system.add_sampler(&sampler2);
    system.add_sampler(&sampler3);
    system.route_sampler(&sampler1, &sampler3);
    system.route_sampler(&sampler2, &sampler3);
    system.route_sampler_to_root(&sampler3);
    system.update(buff.get(), 1);
}

TEST(AudioProcessorTests, TestRoutingDiamond)
{
    //             Sampler 2
    // Sampler 1 <           > root
    //             Sampler 3
    NiceMock<MockSampler> sampler1, sampler2, sampler3, sampler4;
    AudioProcessor system;
    auto buff = unique_ptr<float[]>(new float[1]);

    EXPECT_CALL(sampler1, sample(_, NotNull(), 0, 1)).WillOnce(DoAll(SetArgPointee<1>(1.0f), Return(1)));
    EXPECT_CALL(sampler2, sample(
        Pointee(Pointee(1.0f)), NotNull(), 1, 1
    )).WillOnce(DoAll(SetArgPointee<1>(2.0f), Return(1)));
    EXPECT_CALL(sampler3, sample(
        Pointee(Pointee(1.0f)), NotNull(), 1, 1
    )).WillOnce(DoAll(SetArgPointee<1>(3.0f), Return(1)));
    EXPECT_CALL(sampler4, sample(
        AnyPointeeAtIndices(0, 2, AnyOf(Pointee(2.0f), Pointee(3.0f))),
        NotNull(), 2, 1
    )).Times(1);

    system.add_sampler(&sampler1);
    system.add_sampler(&sampler2);
    system.add_sampler(&sampler3);
    system.add_sampler(&sampler4);
    system.route_sampler(&sampler1, &sampler2);
    system.route_sampler(&sampler1, &sampler3);
    system.route_sampler(&sampler2, &sampler4);
    system.route_sampler(&sampler3, &sampler4);
    system.route_sampler_to_root(&sampler4);
    system.update(buff.get(), 1);
}