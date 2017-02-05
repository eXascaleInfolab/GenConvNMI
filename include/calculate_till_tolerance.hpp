#ifndef GECMI__CALCULATE_TILL_TOLERANCE_HPP_
#define GECMI__CALCULATE_TILL_TOLERANCE_HPP_

#include "vertex_module_maps.hpp"


namespace gecmi {

struct calculated_info_t {
    double empirical_variance;
    double nmi;
};

calculated_info_t calculate_till_tolerance(two_relations_ref two_rel,
    double risk, // <-- Upper bound of probability of the true value being
                  //  -- farthest from estimated value than the epvar
    double epvar,  // Max allowed variance of the result
    bool fasteval=false,  // Approximate (even less accurate), but much faster evaluation
	size_t nds1num=0, size_t nds2num=0  // The number of nodes in the collections
);

} // gecmi

#endif // GECMI__CALCULATE_TILL_TOLERANCE_HPP_
