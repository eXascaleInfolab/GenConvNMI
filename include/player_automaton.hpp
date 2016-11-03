#ifndef GECMI__PLAYER_AUTOMATON_HPP_
#define GECMI__PLAYER_AUTOMATON_HPP_

#include "vertex_module_maps.hpp"


namespace gecmi {

// struct pa_status_t {{{
// Player automaton status
struct pa_status_t {
    enum t {
        SUCCESS,
        EMPTY_SET,
        GOING
    };

    static bool going( pa_status_t::t s1, pa_status_t::t s2 )
    {
        return ( (s1 == SUCCESS ) and ( s2 == GOING ) )
            or
               ( (s1 == GOING   ) and ( s2 == SUCCESS ) )
            or
               ( (s1 == GOING   ) and ( s2 == GOING ) );
    }
}; // }}}

// Represents the state information of a single player
// on the simulation process.
class player_automaton {
    struct pimpl_t;
    pimpl_t *impl;
public:
    player_automaton (remaining_modules_set_t const& rset);

    player_automaton(player_automaton const& other) = delete;

    player_automaton( player_automaton&& other);

    player_automaton& operator=(const player_automaton&) = delete;

    // Called when a player receives a set of modules to operate on.
    void take_set( module_set_t const& mset  );

    // Called to set a common operation
    void set_operation_kind( bool intersect );

    // Called when a coin is tossed and the next operation is decided.
    pa_status_t::t get_status() const;

    size_t get_a_module() const;

    ~player_automaton ();
};

}  // gecmi

#endif // GECMI__PLAYER_AUTOMATON_HPP_

