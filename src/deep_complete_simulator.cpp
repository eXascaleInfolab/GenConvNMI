#include <algorithm>
#include <type_traits>  // remove_reference_t, ...
#include <random>
#include <utility>  // forward
#include <cassert>

#include "representants.hpp"
#include "player_automaton.hpp"
#include "deep_complete_simulator.hpp"


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
    typedef std::mt19937 randgen_t;
    typedef std::mt19937::result_type  gen_seed_t;
    typedef std::uniform_int_distribution<uint32_t>  linear_distrib_t;
    typedef std::vector< importance_float_t > importance_vector_t;

    // For keeping the bi-correspondences; Two vertex to modules bimaps
    two_relations_ref tworel;

    // The random number generator and everything else
    randgen_t rndgen;
    linear_distrib_t  lindis;

    // Input vertices
    vertices_t&  verts;


    pimpl_t( two_relations_ref tworels, vertices_t& vertices, gen_seed_t seed=rd() ):
        tworel( tworels ), rndgen( seed ),
        lindis(0, vertices.size() - 1),
        verts(vertices)  {}

//    ~pimpl_t()
//    {
//        // Used for debugging
//        static size_t num = 0;
//        std::cout << ++num << " ~dcm pimpl" << std::endl;
//    }

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
    }

    simulation_result_t get_sample()
    {
        constexpr static size_t  RESULT_NONE = -1;
        simulation_result_t result(RESULT_NONE);
        uint32_t attempt_count = 0;
        while ( result.first == RESULT_NONE )
        {
            //cout << "-" << endl;
            try_get_sample( result );  // The most heavy function !!!
            if ( result.first == RESULT_NONE )
                result.failed_attempts += result.importance;
            if ( ++attempt_count >= MAX_ACCEPTABLE_FAILURES )
                throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch dcs (maybe your partition is not solvable?)");
        }

        // Note: typically the number of attempts is 1
        //if(attempt_count  > 1)
        //    cout << "Attempts: " <<  attempt_count << endl;
        return result;
    }

    // optional<...> try_get_sample() {{{
    //    This is indeed a huge method.
    void try_get_sample(simulation_result_t& result )  // The most heavy function !!!
    {
        // On the beginning, I need a random shuffle of the vertices, whatever
        // many they be.
        //std::shuffle( verts.begin(), verts.end(), rndgen );  // lindis(rd) wrapper

        // Get the sets of modules (from 2 clusterings/partitions) for the first vertex
        size_t vertex = verts[lindis(rd)];  // 0

        module_set_t rm1, rm2;
        get_modules( vertex, rm1, rm2 );

        // The number of attempts to get modules
        const size_t  attempts = (rm1.size() + rm2.size()) * 2;

        // The automatons that track the state
        //player_automaton pa1(move(rm1)), pa2(move(rm2));
        player_automaton pa1(rm1), pa2(rm2);

        // So, when we have to calculate the probability
        // even on the case that pa1 and pa2 be already
        // set to "ready", I will report the correct
        // weight...
        size_t used_vertex_index = 1;

        // Now is just to draw one by one...if at any moment
        // the system is stuck, try something else...
        while(
            pa_status_t::going( pa1.get_status(), pa2.get_status() )
            //&& used_vertex_index < verts.size()
            && used_vertex_index < attempts
        )
        {
            ++used_vertex_index;

            // Parameters for the second vertex

            //auto iv2 = lindis(rndgen);
            //vertex = verts[ iv2 ];

            // Take modules from clustering 1 or 2 relevant to the origin vertex
            const auto  iv2 = lindis(rd);
            bool  v2first = iv2 % 2;
            module_set_t  v2bms = move(v2first ? rm1 : rm2);  // Base modules for v2
            // Select module (cluster) from which v2 will be selected
            auto  iv2mod = v2bms.begin();
            advance(iv2mod, iv2 % v2bms.size());
            const auto&  mtov = (v2first ? tworel.first : tworel.second).right;
            // Get range of the target vertices from the chosen module (cluster) to select v2
            auto  iverts = mtov.equal_range(*iv2mod);
#ifdef DEBUG
            assert(iverts.first != iverts.second && iverts.first->first == *iv2mod
                && "try_get_sample(), the module must have back relation to the vertex");
#endif // DEBUG
            advance(iverts.first, (iv2 + used_vertex_index)
                % distance(iverts.first, iverts.second));
            vertex = iverts.first->second;  // Get the target vertex

            get_modules( vertex, rm1, rm2 );
            // Now get the operation
            // Note: lindis(rndgen) gives better quality faster than rd()
            bool do_intersection = (iv2 + used_vertex_index) % 2;  // (used_vertex_index + initial_iv2) % 2;  lindis(rndgen) % 2, used_vertex_index % 2
            pa1.set_operation_kind( do_intersection );
            pa2.set_operation_kind( do_intersection );
            pa1.take_set( rm1 );
            pa2.take_set( rm2 );
        }
//        if(used_vertex_index >= 5)
//            printf("%lu ", used_vertex_index);

        importance_float_t  prob = 1.0;

        // Now, if we managed to finish
        if ( pa1.get_status() == pa_status_t::SUCCESS
            && pa2.get_status() == pa_status_t::SUCCESS )
        {
            result.first = pa1.get_a_module();
            result.second = pa2.get_a_module();
            result.importance = prob;
        } else
        {
            result.first = -1;
            result.second = -1;
            result.importance = prob;
        }
    }

}; // pimpl_t

std::random_device deep_complete_simulator::pimpl_t::rd;

// Required for initialization
deep_complete_simulator::deep_complete_simulator( two_relations_ref vmb, vertices_t& verts )
: impl(new pimpl_t(vmb, verts))  {}

// Required for pimpl
deep_complete_simulator::~deep_complete_simulator()
{
    if(impl) {
        delete impl;
        impl = nullptr;
    }
}

size_t deep_complete_simulator::vertices_num() const
{
    return impl ? impl->verts.size() : 0;
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
    return deep_complete_simulator( impl->tworel, impl->verts );
}

}  // gecmi
