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
using std::random_device;

// What's the failure proportion before bailing out... if I get
// at least this many failures, an excpetion will be raised.
constexpr size_t MAX_ACCEPTABLE_FAILURES = 15;  // 127; 1024; 31  // Acceptable number of the subsequently missed vertices

struct deep_complete_simulator::pimpl_t {

    // For random number generation.
    //
    //   I need a random number generator that picks up a random vertex
    //   in the set of remaining vertices.
    //
    static random_device rd;
    static size_t invdrisk;  // Inverted doubled risk (probability the value being outside)

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
        simulation_result_t result;
        uint32_t attempt_count = 0;
        uint32_t failed_attempts = 0;
        while(result.mods1.empty() || result.mods2.empty())
        {
            //cout << "-" << endl;
            try_get_sample( result );  // The most heavy function !!!
            //// Note: exact match provides more accurate results than approximate fuzzy match
            //// and additionally ~ satisfies usecase 1lev4nds
            //// !!! After the proper normalization (importance) the results for hard and soft match are approximately the same !!!
            //if(result.mods1.size() != 1 || result.mods2.size() != 1)
            if(result.mods1.empty() || result.mods2.empty())
                ++failed_attempts;  // += result.importance;
            if ( ++attempt_count >= MAX_ACCEPTABLE_FAILURES ) {
                // Note: it's better to yield approximate results than absence of any results at all
                //throw std::domain_error("SystemIsSuspiciuslyFailingTooMuch dcs (maybe your partition is not solvable?)\n");
                fprintf(stderr, "WARNING get_sample(), the partition is not solvable, results are approximate");
                result.mods1.clear();
                result.mods2.clear();
                result.importance = 0;
                break;
            }
        }

        if(failed_attempts >= 1 && failed_attempts < MAX_ACCEPTABLE_FAILURES)
            result.importance *= importance_float_t(attempt_count - failed_attempts) / attempt_count;
        // Note: typically the number of attempts is 1
        //if(attempt_count  > 1)
        ////    fprintf(stderr, "Attempts: %u\n", attempt_count);
        //    cout << "Attempts: " <<  attempt_count << endl;
        return result;
    }

    // optional<...> try_get_sample() {{{
    //    This is indeed a huge method.
    void try_get_sample(simulation_result_t& result)  // The most heavy function !!!
    {
        // Get the sets of modules (from 2 clusterings/partitions) for the first vertex
        // Note: verts is array of indices
        size_t ivetr;
        size_t vertex;  // = verts[lindis(rndgen)];  // 0, rndgen, rd
        module_set_t rm1, rm2;
        // Note: some vertices might be outlier that are not present in any modules, skip them
        {
            size_t  i = 0;
            const size_t  imax = verts.size();
            do {
                vertex = verts[ivetr = lindis(rndgen)];
                get_modules( vertex, rm1, rm2 );
                // Use vertex that occurs in any module, otherwise take another vertex
            } while(!rm1.size() && !rm2.size() && ++i < imax);
            if(i == imax)
                throw std::runtime_error("try_get_sample(), too many vertices are not members of any module in the input data\n");
        }

        // The number of attempts to get modules
        // Note: full traversing includes all unique vertices in all rm1 and rm2 modules,
        // but with confidence ~ (1 - 0.032 / 2): 31 attempts * modules size is enough
        const size_t  attempts = (rm1.size() + rm2.size()) * invdrisk; // 31; /(risk*2)
        // The automatons that track the state
        //player_automaton pa1(move(rm1)), pa2(move(rm2));
        player_automaton pa1(rm1), pa2(rm2);
        // So, when we have to calculate the probability
        // even on the case that pa1 and pa2 be already
        // set to "ready", I will report the correct
        // weight...
        size_t used_vertex_index = 1;
        // The more common vertex the less it is important. Probability E [0, 1]
        importance_float_t  importance = 1.0 / std::max<size_t>(sqrt(rm1.size() * rm2.size()), 1);
        //const size_t  totmbs = two_rel.first.left.size() + two_rel.second.left.size();
        //importance_float_t  importance = std::max<size_t>(sqrt(rm1.size() * rm2.size()), 1) / totmbs;

        // Now is just to draw one by one...if at any moment
        // the system is stuck, try something else...
        // Note: the projected modules can be only reduced and never grow over the iterations
        while(pa_status_t::going(pa1.get_status(), pa2.get_status())
            //&& used_vertex_index <= verts.size()
            && ++used_vertex_index <= attempts
        ) {
            // Parameters for the second vertex
            static_assert(std::is_integral<decltype(rd())>::value && std::is_unsigned<decltype(rd())>::value
                , "try_get_sample(), rd() value has unexpected type\n");
            // Note: rd() might return the same values on parallel execution, but not lindis.
            const auto  iv2 = lindis(rndgen) ^ rd();  // rndgen, rd
            bool  v2first = iv2 % 2;
            // Take modules from clustering 1 or 2 relevant to the origin vertex
            module_set_t  v2bms = move(v2first ? rm1 : rm2);  // Base modules for v2
            // ATTENTION: a single selected module set can be empty
            // if node base is not synced (differs for the left/right collections)
            if(v2bms.empty()) {
                v2bms = move(v2first ? rm2 : rm1);
                v2first = !v2first;
            }
#ifdef DEBUG
            if(v2bms.empty())
                fprintf(stderr, "try_get_sample(), rm1: %lu, rm2: %lu, uvi: %lu\n"
                    , rm1.size(), rm2.size(), used_vertex_index);
            assert(!v2bms.empty() && "try_get_sample(), both selected module sets shouldn't be empty");
#endif // DEBUG
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
            auto ivt = iverts.first;
            advance(ivt, (iv2 + used_vertex_index) % distance(iverts.first, iverts.second));
            // Do not take the same vertex
            if(ivt->second == vertex && ++ivt == iverts.second)
                ivt = iverts.first;
            // Consider the case of single vertex module(s), which is a RARE case
            if(ivt->second == vertex) {
                // Recover moved rm module(s)
                rm1 = pa1.get_modules();
                rm2 = pa2.get_modules();
                continue;
            }
            vertex = ivt->second;  // Get the target vertex

            get_modules( vertex, rm1, rm2 );
            // Consider early exit for the exact match
            if(rm1.size() == 1 && rm2.size() == 1) {
                //result.importance = 1;  // Exact match
                result.importance = (importance + 1.0) / used_vertex_index;  // 1; Exact match
                result.mods1.assign(rm1.begin(), rm1.end());
                result.mods2.assign(rm2.begin(), rm2.end());
                return;
            }
            // Now get the operation
            // Note: lindis(rndgen) gives better quality faster than rd()
            bool do_intersection = (iv2 + used_vertex_index) % 2;  // (used_vertex_index + initial_iv2) % 2;  lindis(rndgen) % 2, used_vertex_index % 2
            pa1.set_operation_kind( do_intersection );
            pa2.set_operation_kind( do_intersection );
            importance += 1.0 / std::max<size_t>(sqrt(rm1.size() * rm2.size()), 1);  // The more common vertex the less it is important
            pa1.take_set( rm1 );
            pa2.take_set( rm2 );
        }
        // Note: fixed importance = 1 gives very similar results to the importance inverse proportional to the vertex membership
        //result.importance = 1;
        result.importance = importance / used_vertex_index;
//        if(importance <= 2)  // Up to 1 from 1 fixed and 1+ attempting vertices
//            result.importance = importance / 2;  // The more common vertex the less it is important
//        else result.importance = 1;  // There were large enough number of sampled vertices in these modules
        result.mods1.assign(pa1.get_modules().begin(), pa1.get_modules().end());
        result.mods2.assign(pa2.get_modules().begin(), pa2.get_modules().end());
//        //fprintf(stderr, "> try_get_sample(): %lu, %lu (p: %G, failed: %G);  vertex: %lu, cls1: %lu, cls2: %lu\n"
//        //    , result.first, result.second, result.importance, result.failed_attempts, vertex, rm1.size(), rm2.size());
    }

}; // pimpl_t

random_device deep_complete_simulator::pimpl_t::rd;
size_t deep_complete_simulator::pimpl_t::invdrisk = 1 / (0.01 * 2);  // Note: the actual value set later E (0. 1]

void deep_complete_simulator::risk(double r)
{
    assert(r > 0 && r <= 1 && "risk(), The risk value is out of range");
    if(r > 0)
        pimpl_t::invdrisk = 1. / (r * 2);
    else pimpl_t::invdrisk = -1;  // Max value of the unsigned
}

double deep_complete_simulator::risk() noexcept  { return 1. / (pimpl_t::invdrisk * 2); }

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

size_t deep_complete_simulator::vertices_num() const noexcept
{
    return impl ? impl->verts.size() : 0;
}

auto deep_complete_simulator::operator= (deep_complete_simulator&& dcs) noexcept -> deep_complete_simulator&
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
