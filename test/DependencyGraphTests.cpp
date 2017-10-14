#include <soundstone/DependencyGraph.hpp>
#include <gtest/gtest.h>
#include "util/DumbSampler.hpp"

using namespace soundstone;
using namespace soundstone_test;
using namespace std;
using namespace testing;

TEST(DependencyGraphTest, TestBuiltWithNoNodesAttachedToRoot)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.set_parent(&sampler1, &sampler2);
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(0, order.size());
}

TEST(DependencyGraphTest, TestOneItemWithOneDependency)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.attach_to_root(&sampler1);
    graph.set_parent(&sampler1, &sampler2);
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(2, order.size());
    ASSERT_EQ(&sampler2, order[0]->data);
    ASSERT_EQ(&sampler1, order[1]->data);
    ASSERT_EQ(-1, order[0]->dependency_index);
    ASSERT_EQ(0, order[1]->dependency_index);
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
    vector<GraphNode<Sampler> * > order;
    graph.build(order);
    ASSERT_EQ(3, order.size());
    auto *node = order[0];
    ASSERT_TRUE(node->data == &sampler2 || node->data == &sampler3);
    ASSERT_EQ(-1, node->dependency_index);
    node = order[1];
    ASSERT_TRUE(node->data == &sampler2 || node->data == &sampler3);
    ASSERT_EQ(-1, node->dependency_index);
    node = order[2];
    ASSERT_EQ(&sampler1, node->data);
    ASSERT_EQ(1, node->dependency_index);
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
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(4, order.size());
    auto *node = order[0];
    ASSERT_EQ(&sampler4, node->data);
    ASSERT_EQ(-1, node->dependency_index);
    node = order[1];
    ASSERT_TRUE(node->data == &sampler2 || node->data == &sampler3);
    ASSERT_EQ(0, node->dependency_index);
    node = order[2];
    ASSERT_TRUE(node->data == &sampler2 || node->data == &sampler3);
    ASSERT_EQ(0, node->dependency_index);
    node = order[3];
    ASSERT_EQ(&sampler1, node->data);
    ASSERT_EQ(2, node->dependency_index);
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
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(3, order.size());
}

TEST(DependencyGraphTest, TestUnevenGraph)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3, sampler4;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.add(&sampler4);
    graph.attach_to_root(&sampler1);
    graph.attach_to_root(&sampler2);
    graph.attach_to_root(&sampler3);
    graph.set_parent(&sampler2, &sampler4);
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(4, order.size());
    for (const auto *node : order) {
        if (node->data == &sampler1) {
            ASSERT_EQ(-1, node->dependency_index);
        }
        if (node->data == &sampler2) {
            ASSERT_EQ(order[node->dependency_index]->data, &sampler4);
        }
        if (node->data == &sampler3) {
            ASSERT_EQ(-1, node->dependency_index);
        }
        if (node->data == &sampler4) {
            ASSERT_EQ(-1, node->dependency_index);
        }
    }
}

TEST(DependencyGraphTest, TestRemoveItem)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.attach_to_root(&sampler3);
    graph.set_parent(&sampler3, &sampler2);
    graph.set_parent(&sampler2, &sampler1);
    graph.remove(&sampler2);
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(1, order.size());
    auto *node = order[0];
    ASSERT_EQ(&sampler3, node->data);
    ASSERT_EQ(-1, node->dependency_index);
    ASSERT_EQ(0, node->inputs.size());
}

TEST(DependencyGraphTest, TestResurrectItem)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1, sampler2, sampler3;
    graph.add(&sampler1);
    graph.add(&sampler2);
    graph.add(&sampler3);
    graph.attach_to_root(&sampler3);
    graph.set_parent(&sampler3, &sampler2);
    graph.set_parent(&sampler2, &sampler1);
    graph.remove(&sampler2);
    graph.add(&sampler2);
    graph.set_parent(&sampler2, &sampler1);
    vector<GraphNode<Sampler> *> order;
    graph.build(order);
    ASSERT_EQ(1, order.size());
    auto *node = order[0];
    ASSERT_EQ(&sampler3, node->data);
    ASSERT_EQ(-1, node->dependency_index);
    ASSERT_EQ(0, node->inputs.size());
}

TEST(DependencyGraphTest, TestAddSameDataTwice)
{
    DependencyGraph<Sampler> graph;
    DumbSampler sampler1;
    graph.add(&sampler1);
    ASSERT_EXIT(graph.add(&sampler1), KilledBySignal(SIGABRT), "Assertion failed:");
}