#include <soundstone/DependencyGraph.hpp>

#include <stack>
#include <cassert>
#include <algorithm>

using namespace soundstone;
using namespace std;

template <typename T>
void DependencyGraph<T>::add(T data) {
    auto it = _nodes.find(data);
    if (it != _nodes.end()) {
        // Already in graph
        return;
    }

    // Insert a new graph node for this data.
    auto result = _nodes.emplace(
        piecewise_construct,
        forward_as_tuple(data),
        forward_as_tuple()
    );

    // Set the data field for the newly inserted graph node.
    GraphNode<T> &node = result.first->second;
    node.data = data;
}

template <typename T>
void DependencyGraph<T>::remove(T data) {
    // Need to find all references to the data we're removing
    for (auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        GraphNode<T> &node = it->second;

        auto dependency_it = node.dependencies.find(data);
        if (dependency_it != node.dependencies.end()) {
            node.dependencies.erase(dependency_it);
        }
    }

    // Remove the node itself
    auto it = _nodes.find(data);
    if (it != _nodes.end()) {
        _nodes.erase(it);
    }
}

template <typename T>
void DependencyGraph<T>::add_dependency(T dependee, T dependency) {
    auto it = _nodes.find(dependee);
    if (it == _nodes.end()) {
        // Node is not actually in the graph. Can't do anything.
        return;
    }

    // Put the dependency in the node's dependency set.
    it->second.dependencies.emplace(dependency);
}

template <typename T>
void DependencyGraph<T>::remove_dependency(T dependee, T dependency) {
    auto it = _nodes.find(dependee);
    if (it == _nodes.end()) {
        // Node is not actually in the graph. Can't do anything.
        return;
    }

    GraphNode<T> &node = it->second;

    auto dependency_it = node.dependencies.find(dependency);
    if (dependency_it != node.dependencies.end()) {
        node.dependencies.erase(dependency_it);
    }
}

template <typename T>
void DependencyGraph<T>::clear() {
    _nodes.clear();
}

template <typename T>
unique_ptr<T[]> DependencyGraph<T>::resolve(uint32_t &count) const {

    // TODO: Actually resolve correctly

    unique_ptr<T[]> resolved_nodes(new T[_nodes.size()]);

    uint32_t i = 0;
    for (const pair<T, GraphNode<T>> &item_and_node : _nodes) {
        resolved_nodes[i++] = item_and_node.first;
    }

    count = _nodes.size();
    return resolved_nodes;
}


namespace soundstone {
    template class GraphNode<uint32_t>;
    template class DependencyGraph<uint32_t>;
}
