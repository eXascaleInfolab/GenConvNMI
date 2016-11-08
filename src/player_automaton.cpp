#include <iterator>
#include <algorithm>
#include <iostream>

#include <algorithm>  // set_intersection

#include "player_automaton.hpp"


namespace gecmi {

    struct player_automaton::pimpl_t {
        remaining_modules_set_t rms;
        bool operation_is_intersect;

        pimpl_t( remaining_modules_set_t const& rms_ ):
            rms(rms_), operation_is_intersect(false)
        {}

        // void set_operation( bool intersect ){{{
        void set_operation_kind( bool intersect )
        {
            operation_is_intersect = intersect ;
        } // }}}

        // void take_set( module_set_t const& mset ) {{{
        void take_set( module_set_t const& mset )
        {
            if ( rms.size() >= 2 )
            {
                remaining_modules_set_t new_rms;
                // Do the operation
                if ( operation_is_intersect )
                    std::set_intersection(rms.begin(), rms.end(), mset.begin()
                        , mset.end(), std::inserter(new_rms, new_rms.begin()));
                else
                    // So, the new set is the result of the difference
                    // between the old and the taken set.
                    std::set_difference(rms.begin(), rms.end(), mset.begin()
                        , mset.end(), std::inserter(new_rms, new_rms.begin()));

                // Accept the operation only if it doesn't gets to a zero state
                if ( !new_rms.empty() )
                	rms = new_rms;
            }
            // Else do nothing
        } // }}}

        // pa_status_t::t get_status() const {{{
        pa_status_t::t get_status() const
        {
            if ( rms.size() >= 2 )
                return pa_status_t::GOING;

            if ( rms.size() == 0 )
                return pa_status_t::EMPTY_SET;

            return pa_status_t::SUCCESS;
        } // }}}

        size_t get_a_module() const
        {
            assert( rms.size() == 1 );
            return *rms.begin();
        }
    };

    player_automaton::player_automaton(remaining_modules_set_t const& rset)
    : impl(new pimpl_t( rset ) ) {}

    player_automaton::~player_automaton()
    {
        if ( impl )
            delete impl;
    }

    void player_automaton::set_operation_kind( bool intersect )
    {
        impl->set_operation_kind( intersect );
    }

    void player_automaton::take_set( module_set_t const& mset  )
    {
        impl->take_set( mset );
    }

    player_automaton::player_automaton( player_automaton&& other):
        impl( other.impl )
    {
        other.impl = nullptr;
    }

    pa_status_t::t player_automaton::get_status() const
    {
        return impl->get_status();
    }

    size_t player_automaton::get_a_module() const
    {
        return impl->get_a_module();
    }

} // gecmi
