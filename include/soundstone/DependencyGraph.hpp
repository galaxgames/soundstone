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
        unsigned int generation;
        std::vector<GraphNode<T> *> inputs;
        GraphNode<T> *order_list_next;

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
        void build();
        const GraphNode<T> *order() const;
    };

    extern template class GraphNode<Sampler>;
    extern template class DependencyGraph<Sampler>;
}
