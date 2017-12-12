#ifndef GECMI__PARALLEL_WORKER_HPP_
#define GECMI__PARALLEL_WORKER_HPP_

#include <tbb/blocked_range.h>
#include <tbb/spin_mutex.h>

#include "deep_complete_simulator.hpp"


namespace gecmi {

// Second, I will need several instances of the dcs to do
// the work, so fork them... (fork function in deep_complete_simulator
// takes care of doing a deterministic reseed of the randm
// number generator...)
template<typename counter_matrix_ptr>
struct direct_worker {
    deep_complete_simulator dcs_u;
    counter_matrix_ptr const  counter_mat_p;
    tbb::spin_mutex* wait_for_matrix;

    direct_worker( deep_complete_simulator& dcs, counter_matrix_ptr cmp, tbb::spin_mutex* wfm ):
        dcs_u( dcs.fork() ),
        counter_mat_p( cmp ),
        wait_for_matrix( wfm )
    {}

    direct_worker( direct_worker const& other):
        dcs_u( other.dcs_u.fork() ),
        counter_mat_p( other.counter_mat_p ),
        wait_for_matrix( other.wait_for_matrix )
    {}

    direct_worker& operator=(const direct_worker& other) = delete;
//    {
//        dcs_u = other.dcs_u.fork();
//        counter_mat_p = other.counter_mat_p;
//        wait_for_matrix = other.wait_for_matrix;
//    }

    void operator()( const tbb::blocked_range<size_t>& r ) const
    {
        size_t  unmatched = 0;  // The number of unmatched clusters (not solvable)
        for( size_t i=r.begin(); i != r.end(); ++i )
        {
            // Pure and safe memory access to (almost) unrelated
            // locations... (yet contigous, so cache might suffer...)
            //
            simulation_result_t sr = dcs_u.get_sample();

#ifdef DEBUG
            assert(sr.importance >= 0 && "blocked_range(), the importance should be non-negative");
#endif // DEBUG
            if(sr.importance <= 0 || sr.mods1.empty() || sr.mods2.empty()) {
                unmatched += 1;
                continue;
            }
            const importance_float_t prob = sr.importance / (sr.mods1.size() * sr.mods2.size());
            {
                tbb::spin_mutex::scoped_lock l(*wait_for_matrix);
                for(auto m1: sr.mods1)
                    for(auto m2: sr.mods2)
                        (*counter_mat_p)(m1, m2) += prob;
            }
        }

        // Notify about the unmatched clusters
        if(unmatched >= 1)
            fprintf(stderr, "WARNING direct_worker, %lu unmatched (not solvable) clusters met on the collections comparison"
                " (a single cluster could be accounted multiple times)\n", unmatched);
    }
};

}  // gecmi

#endif  //GECMI__PARALLEL_WORKER_HPP_
