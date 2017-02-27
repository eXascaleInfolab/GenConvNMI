#ifndef GECMI__REPRESENTANTS_HPP_
#define GECMI__REPRESENTANTS_HPP_

#include <string>
#include <functional>  // hash, function() wrapper
#include <iostream>

#include "bimap_cluster_populator.hpp"


namespace gecmi {

using std::string;
using std::ostream;


// Extract the modules for the given vertex.
void get_modules(
	size_t vertex,
	vertex_module_bimap_t const& vmb1,
	vertex_module_bimap_t const& vmb2,
	// out
	module_set_t& out_mset1,
	module_set_t& out_mset2
	);

//    static size_t get_vertex_count( vertex_module_bimap_t& vmb );

inline ostream& operator<<( ostream& out, module_set_t const& mset )
{
	out << "(";
	//for ( size_t i: mset ) out << i << " " ;
	for( size_t i: mset )
		out << i << " " ;
	out << " )";
	return out;
}

}  // gecmi

#endif // GECMI__REPRESENTANTS_HPP_

