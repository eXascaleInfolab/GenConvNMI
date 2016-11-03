#ifndef GECMI__DEEP_COMPLETE_SIMULATOR_HPP
#define GECMI__DEEP_COMPLETE_SIMULATOR_HPP

#include <utility>

#include "vertex_module_maps.hpp"
#include "bigfloat.hpp"


namespace gecmi {

struct simulation_result_t {
    size_t first;
    size_t second;
    importance_float_t importance;
    importance_float_t failed_attempts;

    simulation_result_t(
        size_t first,
        size_t second,
        importance_float_t importance,
        importance_float_t failed_attempts
        ):
        first( first ) , second( second ) ,
        importance( importance ),
        failed_attempts(failed_attempts)
    {}

    // Handy for empty init
    simulation_result_t() = default;
};
//typedef std::pair< size_t, size_t > simulation_result_t;

class deep_complete_simulator;

typedef std::unique_ptr< deep_complete_simulator >
    deep_complete_simulator_uptr;

class deep_complete_simulator {
    struct pimpl_t;
    pimpl_t* impl;
public:
    // Required for initialization
    deep_complete_simulator( two_relations_ptr vmb );

    // Forbid copying
    deep_complete_simulator( deep_complete_simulator const& ) = delete;

    deep_complete_simulator& operator= (const deep_complete_simulator&) = delete;

    // For seeding the random number generator
    void set_seed(size_t s);

    // Deterministic fork...
    deep_complete_simulator_uptr fork();

    // Logic here: just get two numbers, a sample from the random
    // variable. The two numbers represent modules.
    simulation_result_t get_sample();

    // Required for pimpl
    ~deep_complete_simulator();
};

}  // gecmi

# endif // GECMI__DEEP_COMPLETE_SIMULATOR_HPP

