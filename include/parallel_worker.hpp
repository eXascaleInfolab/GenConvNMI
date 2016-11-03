#ifndef GECMI__PARALLEL_WORKER_HPP_
#define GECMI__PARALLEL_WORKER_HPP_

#include <tbb/blocked_range.h>
#include <tbb/spin_mutex.h>

#include <boost/numeric/ublas/matrix.hpp>

#include "deep_complete_simulator.hpp"


namespace gecmi {

// struct direct_worker {{{
// Second, I will need several instances of the dcs to do
// the work, so fork them... (fork function in deep_complete_simulator
// takes care of doing a deterministic reseed of the randm
// number generator...)
template<typename counter_matrix_ptr>
struct direct_worker {
    deep_complete_simulator_uptr dcs_u;
    counter_matrix_ptr counter_mat_p;
    tbb::spin_mutex* wait_for_matrix;

    direct_worker( deep_complete_simulator& dcs, counter_matrix_ptr cmp, tbb::spin_mutex* wfm ):
        dcs_u( dcs.fork() ),
        counter_mat_p( cmp ),
        wait_for_matrix( wfm )
    {}

    direct_worker( direct_worker const& other):
        dcs_u( other.dcs_u->fork() ),
        counter_mat_p( other.counter_mat_p ),
        wait_for_matrix( other.wait_for_matrix )
    {}

    direct_worker& operator=(const direct_worker& other) = delete;
//    {
//        dcs_u = other.dcs_u->fork();
//        counter_mat_p = other.counter_mat_p;
//        wait_for_matrix = other.wait_for_matrix;
//    }

    void operator()( const tbb::blocked_range<size_t>& r ) const
    {
        counter_matrix_t& cm = *counter_mat_p;
        for( size_t i=r.begin(); i != r.end(); ++i )
        {
            // Pure and safe memory access to (almost) unrelated
            // locations... (yet contigous, so cache might suffer...)
            //
            simulation_result_t sr = dcs_u->get_sample();

            int i1 = static_cast<int>(sr.first) ;
            int i2 = static_cast<int>(sr.second);
            importance_float_t prob = sr.importance;
            {
                tbb::spin_mutex::scoped_lock l(*wait_for_matrix);
                cm( i1, i2 ) += prob; //
            }
        }
    }
}; // }}}

}  // gecmi

#endif  //GECMI__PARALLEL_WORKER_HPP_
