#include <iostream>

//#include <tbb/task_scheduler_init.h> // <-- For controlling number of working threads
#include "tbb/parallel_for.h"

#include "confusion.hpp"
#include "parallel_worker.hpp"
#include "deep_complete_simulator.hpp"
#include "counts.hpp"
#include "calculate_till_tolerance.hpp"


//constexpr size_t EVCOUNT_THRESHOLD = 8192;
constexpr size_t EVCOUNT_GRAIN = 256;  // 128 -> 1.19;  256 -> 1.08;  512 -> 1.50  // 1024;  // 512

namespace gecmi {

calculated_info_t calculate_till_tolerance(
    two_relations_ptr two_rel,
    double risk , // <-- Upper bound of probabibility of the true value being
                  //  -- farthest from estimated value than the epvar
    double epvar
    )
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;

    uint32_t rows = highest_module_plus_one( two_rel->first );
    uint32_t cols = highest_module_plus_one( two_rel->second );


    counter_matrix_t cm =
        boost::numeric::ublas::zero_matrix< importance_float_t >( rows, cols );

    importance_float_t nmi;
    importance_float_t max_var = 1.0e10;

    deep_complete_simulator dcs( two_rel );

    // Use this to adjust number of threads
    //tbb::task_scheduler_init tsi(1);

    while( epvar < max_var )
    {

        bool raised = false;
        tbb::spin_mutex wait_for_matrix;
        try {
            size_t  steps = rows * cols;
            if(!steps)
                steps = 1;
            parallel_for(
                tbb::blocked_range< size_t >( 0, steps, EVCOUNT_GRAIN ),  // EVCOUNT_THRESHOLD
                direct_worker< counter_matrix_t* >( dcs, &cm, &wait_for_matrix )
            );
        } catch (tbb::tbb_exception const& e) {
            std::cout << "e" << std::endl;
            raised = true;
        }

        if ( raised )
        {
            throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch ctt (maybe your partition is not solvable?)");
        }

        size_t total_events = total_events_from_unmi_cm( cm );
        normalize_events(
            cm,
            norm_conf,
            norm_cols,
            norm_rows
            );
        variances_at_prob(
            norm_conf, norm_cols, norm_rows,
            total_events,
            risk,
            max_var,
            nmi
            );
        //std::cout << total_events << " events simulated. Approx. error: " <<  max_var << std::endl;
    }

    return calculated_info_t{max_var, nmi};
}// calculate_till_tolerance

}  // gecmi
