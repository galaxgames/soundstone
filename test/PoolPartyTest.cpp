#include <soundstone/PoolParty.hpp>

#include <gtest/gtest.h>
#include <chrono>

using namespace soundstone;
using namespace testing;
using namespace std;
using namespace std::chrono;

class PoolPartyTest : public TestWithParam<uint32_t> {

};


TEST(PoolPartyTest, TestConstructDestruct)
{
    PoolParty party;
}

TEST_P(PoolPartyTest, TestWorkWithZeroActions)
{
    PoolParty party;
    party.setup(GetParam());
    party.work();
}

TEST_P(PoolPartyTest, TestWorkWithOneAction)
{
    uint32_t count_a = 0;

    PoolParty party;
    party.setup(GetParam());
    party.add_work([&]{ count_a++; });
    party.work();

    ASSERT_EQ(count_a, 1);
}

TEST_P(PoolPartyTest, TestWorkWithManyActionsWorks)
{
    uint32_t count_a, count_b, count_c, count_d, count_e, count_f;
    count_a = count_b = count_c =count_d = count_e = count_f = 0;

    PoolParty party;
    party.setup(GetParam());
    party.add_work([&]{ count_a++; });
    party.add_work([&]{ count_b++; });
    party.add_work([&]{ count_c++; });
    party.add_work([&]{ count_d++; });
    party.add_work([&]{ count_e++; });
    party.add_work([&]{ count_f++; });
    party.work();

    ASSERT_EQ(count_a, 1);
    ASSERT_EQ(count_b, 1);
    ASSERT_EQ(count_c, 1);
    ASSERT_EQ(count_d, 1);
    ASSERT_EQ(count_e, 1);
    ASSERT_EQ(count_f, 1);
}

TEST_P(PoolPartyTest, TestDependenciesAreRespected)
{
    vector<uint32_t> orders;

    PoolParty party;
    party.setup(GetParam());
    uint32_t dependency = 1;
    party.add_work([&]{ orders.push_back(0); }, &dependency, 1);
    party.add_work([&]{ this_thread::sleep_for(milliseconds(500)); orders.push_back(1); });
    party.work();

    ASSERT_EQ(orders.size(), 2);
    ASSERT_EQ(orders[0], 1);
    ASSERT_EQ(orders[1], 0);
}


INSTANTIATE_TEST_SUITE_P(
    PoolPartyTestImpl,
    PoolPartyTest,
    ::testing::Values(1, 2, 3, 4));
