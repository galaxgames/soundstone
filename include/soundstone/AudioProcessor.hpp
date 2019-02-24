#pragma once
#include "Module.hpp"
#include "SamplerWorker.hpp"
#include "DependencyGraph.hpp"
#include "PoolParty.hpp"
#include <soundstone/RingBuffer.hpp>
#include <soundstone/export.h>

#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <array>


namespace soundstone {

    class SOUNDSTONE_EXPORT AudioProcessor final {

    public:
        class RoutePredicate {
            AudioProcessor *_processor = nullptr;
            Module *_source = nullptr;
        public:
            RoutePredicate(AudioProcessor *processor, Module *source);
            void to(Module *destination, uint32_t index = 0);
        };

    private:
        static const uint32_t MAX_MODULE_INPUTS = 16;

        class ModuleHarness {
        public:
            Module *module = nullptr;
            std::array<Module *, MAX_MODULE_INPUTS> inputs {};
        };

        enum class ActionType {
            ADD_MODULE,
            REMOVE_MODULE,
            ROUTE_MODULE
        };

        class AddRemoveData {
        public:
            Module *module = nullptr;
        };

        class RouteData {
        public:
            Module *source = nullptr;
            Module *dest = nullptr;
            uint32_t index = 0;
        };

        union ActionData {
            AddRemoveData add_remove;
            RouteData route;
        };

        class Action {
        public:
            ActionType type;
            ActionData data;
        };

        std::vector<ModuleHarness> _harnesses;
        std::unordered_map<Module *, uint32_t> _modules_to_harnesses;

        std::vector<std::unique_ptr<float[]>> _sampler_buffers;
        std::vector<std::array<const float *, MAX_MODULE_INPUTS>> _input_vectors;
        std::unique_ptr<float[]> _null_sample_buffer;
        uint32_t _sampler_buffer_length = 0;

        std::vector<std::pair<std::unique_ptr<uint32_t[]>, uint32_t>> _dependency_indices;
        bool _dependency_indices_are_dirty = true;

        PoolParty _party;

        std::queue<Action> _actions;
        std::mutex _actions_mutex;


        void process_actions();
        void process_add(AddRemoveData data);
        void process_remove(AddRemoveData data);
        void process_route(RouteData data);


    public:
        void add_module(Module *module);
        void remove_module(Module *module);

        void set_input(Module *module, uint32_t index, Module *input);
        RoutePredicate route(Module *module);

        void update(uint32_t nsamples);
        void set_thread_count(uint32_t count);
    };
}
