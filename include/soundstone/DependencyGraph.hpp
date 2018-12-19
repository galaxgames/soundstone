#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Sampler.hpp"
#include <soundstone/export.h>

namespace soundstone {

    template <typename T>
    class SOUNDSTONE_EXPORT GraphNode {
    public:
        T *data = nullptr;
        std::vector<std::weak_ptr<GraphNode<T>>> inputs;
        int order_list_index = 0;
        int dependency_index = 0;
    };

    template <typename T>
    class DependencyGraph {
        std::unordered_map<T *, std::shared_ptr<GraphNode<T>>> _nodes;
        GraphNode<T> _root;

    public:
        DependencyGraph();

        void add(T *data);
        void remove(T *data);
        void set_parent(T *child, T *parent);
        void attach_to_root(T* parent);
        void build(std::vector<GraphNode<T> *> &nodes);
        const GraphNode<T>& root() const;
    };

    extern template class GraphNode<Sampler>;
    extern template class DependencyGraph<Sampler>;
}
