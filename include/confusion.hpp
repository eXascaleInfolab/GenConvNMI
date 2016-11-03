#ifndef GECMI__CONFUSION_HPP_
#define GECMI__CONFUSION_HPP_

#include <memory>

#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "bigfloat.hpp"


namespace gecmi {

    typedef boost::numeric::ublas::mapped_matrix< importance_float_t >
        counter_matrix_t;

    typedef boost::numeric::ublas::mapped_matrix< importance_float_t >
        importance_matrix_t;

    typedef boost::numeric::ublas::vector< importance_float_t >
        importance_vector_t;

    typedef std::shared_ptr< counter_matrix_t >
        counter_matrix_ptr;

    void normalize_events( counter_matrix_t const& cm,
        importance_matrix_t& out_norm_conf,
        importance_vector_t& out_norm_cols,
        importance_vector_t& out_norm_rows
        );

    void normalize_events_with_fails( counter_matrix_t const& cm,
        importance_float_t const& fail_count,
        importance_matrix_t& out_norm_conf
        );

    importance_float_t normalized_mi(
        importance_matrix_t const& norm_conf,
        importance_vector_t const& norm_cols,
        importance_vector_t const& norm_rows
    );

    importance_float_t unnormalized_mi(
        importance_matrix_t const& norm_conf,
        importance_vector_t const& out_norm_cols,
        importance_vector_t const& out_norm_rows
    );

    size_t total_events_from_unmi_cm(
        counter_matrix_t const& cm
    );

    void variances_at_prob(
        importance_matrix_t const& norm_conf,
        importance_vector_t const& norm_cols,
        importance_vector_t const& norm_rows,
        size_t total_events,
        double prob,
        double & out_max_variance,
        double & out_nmi
    );

}  // gecmi

#endif // GECMI__CONFUSION_HPP_
