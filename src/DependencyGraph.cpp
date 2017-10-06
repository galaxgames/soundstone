#include <soundstone/DependencyGraph.hpp>
#include <stack>
#include <cassert>

using namespace soundstone;
using namespace std;

template <typename T>
GraphNode<T>::GraphNode() {
    generation = 0;
    order_list_next = nullptr;
}

template <typename T>
DependencyGraph<T>::DependencyGraph() {
}

template <typename T>
void DependencyGraph<T>::add(T *data) {
    // Add a new node to the root of the graph.
    // This can be changed later.
    _nodes.emplace_back(new GraphNode<T>);
    GraphNode<T> *node = _nodes.back().get();
    _nodes_by_data[data] = node;
    node->data = data;
}

template <typename T>
void DependencyGraph<T>::set_parent(T *child, T *parent) {
    GraphNode<T> *child_node = nullptr;
    GraphNode<T> *parent_node = nullptr;
    auto it = _nodes_by_data.find(child);
    if (it != _nodes_by_data.end()) {
        child_node = it->second;
    }
    it = _nodes_by_data.find(parent);
    if (it != _nodes_by_data.end()) {
        parent_node = it->second;
    }

    assert(child_node);
    assert(parent_node);
    assert(find(child_node->inputs.begin(), child_node->inputs.end(), parent_node) == child_node->inputs.end());

    child_node->inputs.push_back(parent_node);
}

template <typename T>
void DependencyGraph<T>::attach_to_root(T *parent) {
    GraphNode<T> *child_node = &_root;
    GraphNode<T> *parent_node = nullptr;
    auto it = _nodes_by_data.find(parent);
    if (it != _nodes_by_data.end()) {
        parent_node = it->second;
    }

    assert(parent_node);
    assert(find(child_node->inputs.begin(), child_node->inputs.end(), parent_node) == child_node->inputs.end());

    child_node->inputs.push_back(parent_node);
}


template <typename T>
void DependencyGraph<T>::build() {

    // Un-set generation of all nodes
    for (unique_ptr<GraphNode<T>> &node : _nodes) {
        node->generation = 0;
    }
    _root.generation = 0;

    vector<GraphNode<T> *> node_stack;
    node_stack.push_back(&_root);
    unsigned int max_gen = 0;

    while (node_stack.size() > 0) {
        GraphNode<T> *current_node = node_stack.back();

        GraphNode<T> *unresolved_parent = nullptr;
        unsigned int youngest_gen = 0;
        for (GraphNode<T> *parent_node : current_node->inputs) {
            // Check for circular dependency
            if (find(node_stack.begin(), node_stack.end(), parent_node) != node_stack.end()) {
                // TODO: Warn of circular dependency
            }
            else {
                unsigned int parent_gen = parent_node->generation;
                if (parent_gen == 0) {
                    unresolved_parent = parent_node;
                    break;
                }
                youngest_gen = max(youngest_gen, parent_gen);
            }
        }

        if (unresolved_parent != nullptr) {
            node_stack.push_back(unresolved_parent);
            continue;
        }

        // All parents are resolved, we can determine the correct generation
        // for this node.
        unsigned int gen = youngest_gen + 1;
        current_node->generation = gen;
        max_gen = max(max_gen, gen);
        node_stack.pop_back();
    }

    // Decrement max gen as the root node does not count.
    --max_gen;

    if (max_gen == 0) {
        // early out
        _root.order_list_next = nullptr;
        return;
    }

    // Build nodes by generation
    vector<vector<GraphNode<T> *>> nodes_by_generation;
    nodes_by_generation.resize(max_gen);
    for (unique_ptr<GraphNode<T>> &node : _nodes) {
        if (node->generation != 0) {
            assert(node->generation > 0 && node->generation <= max_gen);
            vector<GraphNode<T> *> &gen_list = nodes_by_generation[node->generation - 1];
            gen_list.push_back(node.get());
        }
    }

    // Build intrusive order list
    GraphNode<T> *previous = &_root;
    for (size_t i = nodes_by_generation.size(); i > 0; --i) {
        const vector<GraphNode<T> *> &gen_list = nodes_by_generation[i - 1];
        for (GraphNode<T> *node : gen_list) {
            previous->order_list_next = node;
            previous = node;
        }
    }
    previous->order_list_next = nullptr;

    // I think that's it.
}

template <typename T>
const GraphNode<T> *DependencyGraph<T>::order() const {
    return _root.order_list_next;
}

namespace soundstone {
    template class GraphNode<Sampler>;
    template class DependencyGraph<Sampler>;

}