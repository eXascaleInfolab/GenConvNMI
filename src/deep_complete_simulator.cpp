// includes {{{
#include <algorithm>
//#include <iostream> // For debug
#include <type_traits>  // remove_reference_t, ...
#include <random>
#include <cassert>

//#include <boost/random/mersenne_twister.hpp>  // mt19937
//#include <boost/random/uniform_smallint.hpp>
//#include <boost/random/variate_generator.hpp>
//#include <boost/optional.hpp>

#include "representants.hpp"
#include "player_automaton.hpp"
#include "deep_complete_simulator.hpp"
// }}}


namespace gecmi {

using std::cout;
using std::endl;

//size_t __i_count = 0;
//size_t __d_count = 0;

//struct __print{
    //~__print() {
        //cout << "__i_count " << __i_count << endl;
        //cout << "__d_count " << __d_count << endl;
    //};
//};

// What's the failure proportion before bailing out... if I get
// at least this many failures, an excpetion will be raised.
constexpr size_t MAX_ACCEPTABLE_FAILURES = 63;

//__print p;

// struct simulation_inner_result_t {{{
struct simulation_inner_result_t {
    int first;
    int second;
    importance_float_t importance;

    simulation_inner_result_t(
        size_t first, size_t second,
        importance_float_t importance
    ):
        first( first ) , second( second ) , importance( importance )
    {
    }

    // Handy for empty init
    simulation_inner_result_t(): first(0), second(0), importance(0)  {}
}; // }}}

struct deep_complete_simulator::pimpl_t {

    // For random number generation.
    //
    //   I need a random number generator that picks up a random vertex
    //   in the set of remaining vertices.
    //
    static std::random_device rd;
    typedef std::mt19937 base_generator_type;
    typedef std::uniform_int_distribution<uint32_t>  coin_distribution_type;
//    typedef boost::random::uniform_smallint<> coin_distribution_type;
//    typedef boost::variate_generator<base_generator_type&, coin_distribution_type> gen_type;
//    typedef base_generator_type  gen_type;
    typedef std::vector< importance_float_t > importance_vector_t;

    //representant2appereances_map_t repr2apprs;

    // For keeping the bi-correspondences; Vertex to modules bimaps
    two_relations_ref tworel;

    // The random number generator and everything else
    base_generator_type rnd_generator;
//    coin_distribution_type  coin_distrib;

    // How many vertices there are
//    size_t vertex_count;
    std::vector< size_t >  vertices;


    // pimpl_t( two_relations_ptr vmb_ ) {{{
    pimpl_t( two_relations_ref tworels ):
        tworel( tworels ), rnd_generator( rd() ),
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
        assert(vertices.size() == get_vertex_count( tworel.first )
            && "The vertices both clusterings should be the same");
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
        simulation_inner_result_t inner_result;
        inner_result.first = -1;
        importance_float_t failed = 0;
        uint32_t attempt_count = 0;
        while ( inner_result.first == -1 )
        {
            //cout << "-" << endl;
            try_get_sample( inner_result );
            if ( inner_result.first == -1 )
            {
                failed += inner_result.importance;
            }
            attempt_count ++;
            if ( attempt_count >= MAX_ACCEPTABLE_FAILURES )
            {
                throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch dcs (maybe your paritition is not solvable?)");
            }
        }
        simulation_result_t result(
            inner_result.first, inner_result.second,
            inner_result.importance, failed );

        return result;
    } // }}}

//    struct lambda_0 {
//        base_generator_type& rnd_generator;
//        coin_distribution_type vertex_dist;
//
//        lambda_0( base_generator_type& rnd_generator_, size_t N ):
//            rnd_generator( rnd_generator_ ), vertex_dist(0, N-1)  {}
//
//        size_t operator()()
//        {
//            return vertex_dist(rnd_generator);
//        }
//    };

    // optional<...> try_get_sample() {{{
    //    This is indeed a huge method.
    void try_get_sample(simulation_inner_result_t & result )
    {
        //coin_distribution_type dt; // <-- Construction without arguments
//        lambda_0 vertex_gen( rnd_generator, vertices.size() );

        // On the beginning, I need a random shuffle of the vertices, whatever
        // many they be.
//        std::vector< size_t > vertices( vertex_count );
//        for(size_t i=0, vcount = vertices.size(); i < vcount; ++i)
//            vertices[i] = i;
        std::shuffle( vertices.begin(), vertices.end(), rnd_generator );

        // Get the sets of modules of the first vertex
        size_t vertex = vertices[0];

        // DEBUG
        //if ( vertex ==0 )
        //{
            //__i_count ++;
        //} else
        //{
            //__d_count ++;
        //}
        //bool first_is_difference = false;
        // end DEBUG

        module_set_t rm1, rm2;
        get_modules( vertex, rm1, rm2 );

        // The automatons that track the state
        player_automaton pa1( rm1 ), pa2(rm2);

        // So, when we have to calculate the probability
        // even on the case that pa1 and pa2 be already
        // set to "ready", I will report the correct
        // weight...
        size_t used_vertex_index = 1;

        // The representant tracker
        //representant_tracker rtracker( rm1, rm2 );

        coin_distribution_type coin_dt( 0, 1 );// This and the previous
                 // object are supposed to be lightweight...
        //gen_type coin_gen( rnd_generator, coin_dt );

        // Now is just to draw one by one...if at any moment
        // the system is stuck, try something else...
        while(
            pa_status_t::going( pa1.get_status(),pa2.get_status() )
            && used_vertex_index < vertices.size()
        )
        {
            //size_t remaining_vertices = vertex_count - used_vertex_index ;
            vertex = vertices[ used_vertex_index ++ ];
            get_modules( vertex, rm1, rm2 );
            // Now get the operation
            bool do_intersection = ( coin_dt(rd) == 0 );
            pa1.set_operation_kind( do_intersection );
            pa2.set_operation_kind( do_intersection );
            pa1.take_set( rm1 );
            pa2.take_set( rm2 );
        }

        importance_float_t
            prob = 1.0;

        // Now, if we managed to finish
        if ( ( pa1.get_status() == pa_status_t::SUCCESS) and
             ( pa2.get_status() == pa_status_t::SUCCESS) )
        {
            result.first = pa1.get_a_module();
            result.second = pa2.get_a_module();
            //if ( starts_in_2 and result.first == 2 and result.second==2 )
            //{
                //cout << "p prob " << prob << endl;
                //__i_count ++ ;
            //} else
            //{
                //__d_count ++ ;
            //}
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

//// For seeding the random number generator
//void deep_complete_simulator::set_seed(size_t s) const
//{
//    impl->rnd_generator.seed( s );
//}

simulation_result_t deep_complete_simulator::get_sample() const
{
    return impl->get_sample();
}

// Deterministic fork...
deep_complete_simulator deep_complete_simulator::fork() const
{
    // Advance a litle bit the generator
//    static_assert(std::is_integral<decltype(seed_gen())>::value
//        , "The seed generator should yield integral values");
//    static_assert(std::is_unsigned<decltype(seed_gen())>::value
//        , "The seed generator should yield unsigned values");

    //deep_complete_simulator result( impl->tworel );
    //result.set_seed( pimpl_t::coin_distribution_type()(pimpl_t::rd) );
    return deep_complete_simulator( impl->tworel );
}

}  // gecmi
