#include <soundstone/DependencyGraph.hpp>
#include <stack>
#include <cassert>

using namespace soundstone;
using namespace std;

template <typename T>
GraphNode<T>::GraphNode() {
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
void DependencyGraph<T>::build(vector<GraphNode<T> *> &nodes_in_order) {

    // Un-set index of all nodes
    for (unique_ptr<GraphNode<T>> &node : _nodes) {
        node->order_list_index = -1;
    }
    _root.order_list_index = -1;

    vector<GraphNode<T> *> node_stack;
    node_stack.push_back(&_root);
    unsigned int max_gen = 0;
    nodes_in_order.clear();

    while (!node_stack.empty()) {
        GraphNode<T> *current_node = node_stack.back();

        GraphNode<T> *unresolved_parent = nullptr;
        int youngest_parent_dependency = -1;
        for (GraphNode<T> *parent_node : current_node->inputs) {
            // Check for circular dependency
            if (find(node_stack.begin(), node_stack.end(), parent_node) != node_stack.end()) {
                // TODO: Warn of circular dependency
            }
            else {
                int parent_index = parent_node->order_list_index;
                if (parent_index == -1) {
                    unresolved_parent = parent_node;
                    break;
                }
                youngest_parent_dependency = max(youngest_parent_dependency, parent_index);
            }
        }

        if (unresolved_parent != nullptr) {
            node_stack.push_back(unresolved_parent);
            continue;
        }

        // All parents are resolved
        current_node->order_list_index = static_cast<int>(nodes_in_order.size());
        current_node->dependency_index = youngest_parent_dependency;
        if (current_node != &_root) {
            nodes_in_order.push_back(current_node);
        }
        node_stack.pop_back();
    }
}

template <typename T>
const GraphNode<T> &DependencyGraph<T>::root() const {
    return _root;
}

namespace soundstone {
    template class GraphNode<Sampler>;
    template class DependencyGraph<Sampler>;
}
