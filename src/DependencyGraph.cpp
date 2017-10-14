#include <soundstone/DependencyGraph.hpp>
#include <stack>
#include <cassert>
#include <algorithm>

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
    // Add a new node to the graph, unattached to anything.
    assert(_nodes.find(data) == _nodes.end());
    auto *node = new GraphNode<T>;
    _nodes.emplace(piecewise_construct, forward_as_tuple(data), forward_as_tuple(node));
    node->data = data;
}

template <typename T>
void DependencyGraph<T>::remove(T *data)  {
    _nodes.erase(data);
}

template <typename T>
void DependencyGraph<T>::set_parent(T *child, T *parent) {
    shared_ptr<GraphNode<T>> child_node;
    shared_ptr<GraphNode<T>> parent_node;
    auto it = _nodes.find(child);
    if (it != _nodes.end()) {
        child_node = it->second;
    }
    it = _nodes.find(parent);
    if (it != _nodes.end()) {
        parent_node = it->second;
    }


    assert(child_node != nullptr);
    assert(parent_node != nullptr);
#if !NDEBUG
    for (const auto &input : child_node->inputs) {
        assert(input.lock() != parent_node);
    }
#endif

    child_node->inputs.push_back(parent_node);
}

template <typename T>
void DependencyGraph<T>::attach_to_root(T *parent) {
    GraphNode<T> *child_node = &_root;
    shared_ptr<GraphNode<T>> parent_node;
    auto it = _nodes.find(parent);
    if (it != _nodes.end()) {
        parent_node = it->second;
    }

    assert(parent_node);
#if !NDEBUG
    for (const auto &input : child_node->inputs) {
        assert(input.lock() != parent_node);
    }
#endif

    child_node->inputs.push_back(parent_node);
}


template <typename T>
void DependencyGraph<T>::build(vector<GraphNode<T> *> &nodes_in_order) {

    // Un-set index of all nodes
    for (auto &pair : _nodes) {
        pair.second->order_list_index = -1;
    }
    _root.order_list_index = -1;

    vector<GraphNode<T> *> node_stack;
    vector<size_t> dead_children;
    node_stack.push_back(&_root);
    unsigned int max_gen = 0;
    nodes_in_order.clear();

    while (!node_stack.empty()) {
        GraphNode<T> *current_node = node_stack.back();

        GraphNode<T> *unresolved_parent = nullptr;
        int youngest_parent_dependency = -1;
        dead_children.clear();

        vector<weak_ptr<GraphNode<T>>> &inputs = current_node->inputs;
        for (size_t i = 0, ilen = inputs.size(); i < ilen; ++i) {
            shared_ptr<GraphNode<T>> parent_node = inputs[i].lock();
            if (parent_node == nullptr) {
                // This is a removed node.
                dead_children.push_back(i);
                continue;
            }

            // Check for circular dependency
            if (find(node_stack.begin(), node_stack.end(), parent_node.get()) != node_stack.end()) {
                // TODO: Warn of circular dependency
            }
            else {
                int parent_index = parent_node->order_list_index;
                if (parent_index == -1) {
                    unresolved_parent = parent_node.get();
                    break;
                }
                youngest_parent_dependency = max(youngest_parent_dependency, parent_index);
            }
        }

        // Remove dead inputs.
        for (auto it = dead_children.rbegin(); it != dead_children.rend(); ++it) {
            inputs.erase(inputs.begin() + *it);
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
