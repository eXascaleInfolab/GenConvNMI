// includes {{{
#include <algorithm>
//#include <iostream> // For debug
#include <type_traits>  // remove_reference_t, ...
#include <random>
#include <cassert>

#include "representants.hpp"
#include "player_automaton.hpp"
#include "deep_complete_simulator.hpp"
// }}}


namespace gecmi {

using std::cout;
using std::endl;


// What's the failure proportion before bailing out... if I get
// at least this many failures, an excpetion will be raised.
constexpr size_t MAX_ACCEPTABLE_FAILURES = 31;

struct deep_complete_simulator::pimpl_t {

    // For random number generation.
    //
    //   I need a random number generator that picks up a random vertex
    //   in the set of remaining vertices.
    //
    static std::random_device rd;
    typedef std::mt19937 base_generator_type;
    typedef std::mt19937::result_type  gen_seed_t;
    //typedef std::uniform_int_distribution<uint32_t>  coin_distribution_type;
    typedef std::vector< importance_float_t > importance_vector_t;

    // For keeping the bi-correspondences; Vertex to modules bimaps
    two_relations_ref tworel;

    // The random number generator and everything else
    base_generator_type rnd_generator;

    // How many vertices there are
    std::vector< size_t >  vertices;


    // pimpl_t( two_relations_ptr vmb_ ) {{{
    pimpl_t( two_relations_ref tworels, gen_seed_t seed=rd() ):
        tworel( tworels ), rnd_generator( seed ),
        //vertex_count( get_vertex_count( vmb_.first ) )
        //repr2apprs( extract_representants( vmb_->first, vmb_->second ) )
        vertices()
    {
        vertices.reserve( get_vertex_count( tworel.first ) );
        auto& vmap = tworel.first.left;  // First vmap
        // Fill the vertices
        for(const auto& ind = vmap.begin(); ind != vmap.end();) {
            vertices.push_back(ind->first);
            const_cast<typename std::remove_reference_t<decltype(vmap)>::iterator&>(ind)
                = vmap.equal_range(ind->first).second;
        }
#ifdef DEBUG
        assert(vertices.size() == get_vertex_count( tworel.first )
            && "The vertices both clusterings should be the same");
#endif  // DEBUG
    } // }}}

    // void get_modules( size_t vertex, module_set_t& mset1, module_set_t& mset2) const {{{
    // Given a vertex, populate two sets of modules
    // with the corresponding modules according to
    // each correspondence.
    void get_modules( size_t vertex, module_set_t& mset1, module_set_t& mset2) const
    {
        mset1.clear();
        mset2.clear();

        gecmi::get_modules(
            vertex,
            tworel.first,
            tworel.second,
            mset1,
            mset2 );
    } // }}}

    // simulation_result_t get_sample() {{{
    simulation_result_t get_sample()
    {
        simulation_result_t result(-1);
        uint32_t attempt_count = 0;
        while ( result.first == -1 )
        {
            //cout << "-" << endl;
            try_get_sample( result );  // The most heavy function !!!
            if ( result.first == -1 )
                result.failed_attempts += result.importance;
            if ( ++attempt_count >= MAX_ACCEPTABLE_FAILURES )
                throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch dcs (maybe your paritition is not solvable?)");
        }

        // Note: typically the number of attempts is 1
        //if(attempt_count  > 1)
        //    cout << "Attempts: " <<  attempt_count << endl;
        return result;
    } // }}}

    // optional<...> try_get_sample() {{{
    //    This is indeed a huge method.
    void try_get_sample(simulation_result_t& result )  // The most heavy function !!!
    {
        //coin_distribution_type coin_dt( 0, 1 );// This and the previous
        // object are supposed to be lightweight...

        // On the beginning, I need a random shuffle of the vertices, whatever
        // many they be.
        std::shuffle( vertices.begin(), vertices.end(), rnd_generator );

        // Get the sets of modules of the first vertex
        size_t vertex = vertices[0];

        module_set_t rm1, rm2;
        get_modules( vertex, rm1, rm2 );

        // The automatons that track the state
        player_automaton pa1( rm1 ), pa2(rm2);

        // So, when we have to calculate the probability
        // even on the case that pa1 and pa2 be already
        // set to "ready", I will report the correct
        // weight...
        size_t used_vertex_index = 1;

        // Now is just to draw one by one...if at any moment
        // the system is stuck, try something else...
        while(
            pa_status_t::going( pa1.get_status(), pa2.get_status() )
            && used_vertex_index < vertices.size()
        )
        {
            vertex = vertices[ used_vertex_index ++ ];
            get_modules( vertex, rm1, rm2 );
            // Now get the operation
            bool do_intersection = rnd_generator() % 2;  // coin_dt(rd);
            pa1.set_operation_kind( do_intersection );
            pa2.set_operation_kind( do_intersection );
            pa1.take_set( rm1 );
            pa2.take_set( rm2 );
        }

        importance_float_t
            prob = 1.0;

        // Now, if we managed to finish
        if ( pa1.get_status() == pa_status_t::SUCCESS
            && pa2.get_status() == pa_status_t::SUCCESS )
        {
            result.first = pa1.get_a_module();
            result.second = pa2.get_a_module();
            result.importance = prob ;
        } else
        {
            result.first = -1;
            result.second = -1;
            result.importance = prob ;
        }
    } // }}}

}; // pimpl_t

std::random_device deep_complete_simulator::pimpl_t::rd;

// Required for initialization
deep_complete_simulator::deep_complete_simulator( two_relations_ref vmb )
    : impl( new pimpl_t( vmb ) )  {}

// Required for pimpl
deep_complete_simulator::~deep_complete_simulator()
{
    if(impl) {
        delete impl;
        impl = nullptr;
    }
}

auto deep_complete_simulator::operator= (deep_complete_simulator&& dcs) -> deep_complete_simulator&
{
    if(impl)
        delete impl;
    impl = dcs.impl;
    dcs.impl = nullptr;

    return *this;
}

simulation_result_t deep_complete_simulator::get_sample() const
{
    return impl->get_sample();
}

// Deterministic fork...
deep_complete_simulator deep_complete_simulator::fork() const
{
    return deep_complete_simulator( impl->tworel );
}

}  // gecmi
