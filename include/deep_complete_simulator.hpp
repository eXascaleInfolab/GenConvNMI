#ifndef GECMI__DEEP_COMPLETE_SIMULATOR_HPP
#define GECMI__DEEP_COMPLETE_SIMULATOR_HPP

#include "vertex_module_maps.hpp"
#include "bigfloat.hpp"


namespace gecmi {

struct simulation_result_t {
    size_t first;
    size_t second;
    importance_float_t importance;
    importance_float_t failed_attempts;

    simulation_result_t(size_t first=0, size_t second=0,
        importance_float_t importance=0,
        importance_float_t failed_attempts=0
    ): first(first), second(second), importance(importance),
        failed_attempts(failed_attempts)  {}
};

class deep_complete_simulator {
    struct pimpl_t;
    pimpl_t* impl;
public:
    // Required for initialization
    deep_complete_simulator(two_relations_ref vmb, vertices_t& verts);

    // Required for pimpl
    ~deep_complete_simulator();

    deep_complete_simulator(deep_complete_simulator&& dcs) = default;
    deep_complete_simulator& operator= (deep_complete_simulator&&);

    // Forbid copying
    deep_complete_simulator( deep_complete_simulator const& ) = delete;

    deep_complete_simulator& operator= (const deep_complete_simulator&) = delete;

    // Deterministic fork...
    deep_complete_simulator fork() const;

    // Logic here: just get two numbers, a sample from the random
    // variable. The two numbers represent modules.
    simulation_result_t get_sample() const;

    size_t vertices_num() const;
};

}  // gecmi

# endif // GECMI__DEEP_COMPLETE_SIMULATOR_HPP

