#include <iterator>
#include <algorithm>  // set_intersection

#include "representants.hpp"


namespace gecmi{

    void get_modules(
        size_t vertex,
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2,
        // out
        module_set_t& out_mset1,
        module_set_t& out_mset2
        )
    {
        // A map from vertex (a single one) to the set of modules

        // Get the modules 1
        auto range = vmb1.left.equal_range(vertex);
        for(auto ivm = range.first; ivm != range.second; ++ivm)
			out_mset1.insert(ivm->second);

        // Get the modules 2
        range = vmb2.left.equal_range(vertex);
        for(auto ivm = range.first; ivm != range.second; ++ivm)
			out_mset2.insert(ivm->second);
    }

//    // size_t get_vertex_count( vertex_module_bimap_t const& vmb ) {{{
//    size_t get_vertex_count( vertex_module_bimap_t& vmb )
//    {
//    	return uniqSize(vmb.left);
//    } // }}}

}  // gecmi
