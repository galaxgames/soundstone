#pragma once
#include <soundstone/export.h>
#include "Module.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>


namespace soundstone {

    template <typename T>
    class SOUNDSTONE_EXPORT GraphNode {
    public:
        T data = {};
        std::unordered_set<T> dependencies;
    };

    template <typename T>
    class SOUNDSTONE_EXPORT DependencyGraph {
        std::unordered_map<T, GraphNode<T>> _nodes;

    public:
        void add(T data);
        void remove(T data);
        void add_dependency(T dependee, T dependency);
        void remove_dependency(T dependee, T dependency);
        void clear();
        std::unique_ptr<T[]> resolve(uint32_t &count) const;

    };

    extern template class GraphNode<uint32_t>;
    extern template class DependencyGraph<uint32_t>;
}
