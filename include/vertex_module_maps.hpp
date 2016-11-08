#ifndef GECMI__VERTEX_MODULE_MAPS_HPP_
#define GECMI__VERTEX_MODULE_MAPS_HPP_

#include <memory>
#include <set>
#include <vector>
#include <utility>

#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/bimap.hpp>


namespace gecmi {

using namespace boost::bimaps;

// Normally vertices are assumed to be in the left, modules in the right.
// What happens if the network is swapped? There is a small chance that
// some branches have to be discarded.
typedef bimap< unordered_multiset_of<size_t>, unordered_multiset_of<size_t> >
    vertex_module_bimap_t;

typedef vertex_module_bimap_t::relation relation_t;

typedef std::pair< vertex_module_bimap_t, vertex_module_bimap_t >
    two_relations_t;

typedef two_relations_t&  two_relations_ref;

typedef std::set< size_t > module_set_t;  // ATTENTIOM: must be an ORDERED container
//typedef std::set<size_t> remaining_modules_set_t; <-- Now just an alias for
// the same thing.

typedef module_set_t  modules_set_t;
typedef module_set_t  remaining_modules_set_t;

typedef std::vector< size_t >  vertices_t;


// I need a couple of bimaps to reflect to full thing structure...

}  // gecmi

#endif // GECMI__VERTEX_MODULE_MAPS_HPP_

