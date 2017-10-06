#include <soundstone/DependencyGraph.hpp>
#include <gtest/gtest.h>
#include "DumbSampler.hpp"

using namespace soundstone;
using namespace soundstone_test;
using namespace std;

TEST(DependencyGraphTest, InitialOrderFrontIsNull)
{
    DependencyGraph<Sampler> graph;
    ASSERT_EQ(nullptr, graph.order());
}

TEST(DependencyGraphTest, TestBuiltWithNoNodesAttachedToRoot)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.set_parent(&sampler1, &sampler2);
    graph.build();
    auto *order = graph.order();
    ASSERT_EQ(nullptr, order);
}

TEST(DependencyGraphTest, TestOneItemWithOneDependency)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.attach_to_root(&sampler1);
    graph.set_parent(&sampler1, &sampler2);
    graph.build();
    auto *order = graph.order();
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler1, order->data);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler2, order->data);
    order = order->order_list_next;
    ASSERT_EQ(nullptr, order);
}

TEST(DependencyGraphTest, TestOneItemWithTwoDependencies)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.attach_to_root(&sampler1);
    graph.set_parent(&sampler1, &sampler2);
    graph.set_parent(&sampler1, &sampler3);
    graph.build();
    auto *order = graph.order();
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler1, order->data);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_TRUE(order->data == &sampler2 || order->data == &sampler3);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_TRUE(order->data == &sampler2 || order->data == &sampler3);
    order = order->order_list_next;
    ASSERT_EQ(nullptr, order);
}

TEST(DependencyGraphTest, TestDiamond)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3, sampler4;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.add(&sampler4);
    graph.attach_to_root(&sampler1);
    graph.set_parent(&sampler1, &sampler2);
    graph.set_parent(&sampler1, &sampler3);
    graph.set_parent(&sampler2, &sampler4);
    graph.set_parent(&sampler3, &sampler4);
    graph.build();
    auto *order = graph.order();
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler1, order->data);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_TRUE(order->data == &sampler2 || order->data == &sampler3);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_TRUE(order->data == &sampler2 || order->data == &sampler3);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler4, order->data);
    order = order->order_list_next;
    ASSERT_EQ(nullptr, order);
}

TEST(DependencyGraphTest, TestCircularDependency)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.attach_to_root(&sampler1);
    graph.set_parent(&sampler1, &sampler2);
    graph.set_parent(&sampler2, &sampler3);
    graph.set_parent(&sampler3, &sampler1);
    graph.build();
    auto *order = graph.order();
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler1, order->data);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler2, order->data);
    order = order->order_list_next;
    ASSERT_NE(nullptr, order);
    ASSERT_EQ(&sampler3, order->data);
    order = order->order_list_next;
    ASSERT_EQ(nullptr, order);
}