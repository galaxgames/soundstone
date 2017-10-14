#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Sampler.hpp"

namespace soundstone {

    template <typename T>
    class GraphNode {
    public:
        T *data;
        std::vector<GraphNode<T> *> inputs;
        int order_list_index;
        int dependency_index;
        GraphNode();
    };

    template <typename T>
    class DependencyGraph {
        std::vector<std::unique_ptr<GraphNode<T>>> _nodes;
        std::unordered_map<T *, GraphNode<T> *> _nodes_by_data;
        GraphNode<T> _root;

    public:
        DependencyGraph();

        void add(T *data);
        void set_parent(T *child, T *parent);
        void attach_to_root(T* parent);
        void build(std::vector<GraphNode<T> *> &nodes);
        const GraphNode<T>& root() const;
    };

    extern template class GraphNode<Sampler>;
    extern template class DependencyGraph<Sampler>;
}
