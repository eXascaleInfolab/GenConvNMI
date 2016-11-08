#include <iostream>

#include <tbb/task_scheduler_init.h> // <-- For controlling number of working threads
#include <tbb/parallel_for.h>

#include "bimap_cluster_populator.hpp"
#include "confusion.hpp"
#include "parallel_worker.hpp"
#include "deep_complete_simulator.hpp"
#include "calculate_till_tolerance.hpp"


//constexpr size_t  EVCOUNT_THRESHOLD = 8192;  // Gives also good results on middle-size networks
// Grain: 1024 .. 2048;  1536 is ~ the best on both middle-size and large networks
// on the working laptop (4 cores Intel(R) Core(TM) i7-4600U CPU @ 2.10GHz-3.3 GHz)
// Checked in the range 128 .. 8192
// Note: it has not significant dependence on the taks complexity: the same value is
// optimal using vector instantiation and shuffling
constexpr size_t  EVCOUNT_GRAIN = 1536;

namespace gecmi {

calculated_info_t calculate_till_tolerance(
    two_relations_ref two_rel,
    double risk , // <-- Upper bound of probabibility of the true value being
                  //  -- farthest from estimated value than the epvar
    double epvar
    )
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;

    // left: Nodes, right: Clusters
    size_t rows = uniqSize(two_rel.first.right) + 1;
    size_t cols = uniqSize(two_rel.second.right) + 1;

    counter_matrix_t cm =
        boost::numeric::ublas::zero_matrix< importance_float_t >( rows, cols );

    importance_float_t nmi;
    importance_float_t max_var = 1.0e10;


    vertices_t  vertices;
    vertices.reserve( uniqSize( two_rel.first.left ) );
    auto& vmap = two_rel.first.left;  // First vmap
    // Fill the vertices
    for(const auto& ind = vmap.begin(); ind != vmap.end();) {
        vertices.push_back(ind->first);
        const_cast<typename std::remove_reference_t<decltype(vmap)>::iterator&>(ind)
            = vmap.equal_range(ind->first).second;
    }
#ifdef DEBUG
    assert(vertices.size() == uniqSize( two_rel.second.left )
        && "The vertices both clusterings should be the same");
#endif  // DEBUG

    deep_complete_simulator dcs(two_rel, vertices);

    // The expected number of communities should not increase the square root from the number of nodes
    // Note: such definition should yield faster computation when the number of clusters is huge
    // and their size is small (SNAP Amazon dataset)
    const size_t  steps = (rows - 1) * (cols - 1);  // Process all cells of the table

    // Use this to adjust number of threads
    tbb::task_scheduler_init tsi;

    while( epvar < max_var )
    {

        tbb::spin_mutex wait_for_matrix;
        try {
            parallel_for(
                tbb::blocked_range< size_t >( 0, steps, EVCOUNT_GRAIN ),  // EVCOUNT_THRESHOLD
                direct_worker< counter_matrix_t* >( dcs, &cm, &wait_for_matrix )
            );
        } catch (tbb::tbb_exception const& e) {
            std::cout << "e" << std::endl;
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
