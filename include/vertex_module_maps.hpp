#ifndef GECMI__VERTEX_MODULE_MAPS_HPP_
#define GECMI__VERTEX_MODULE_MAPS_HPP_

#include <memory>
#include <set>
#include <utility>

#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/bimap.hpp>


namespace gecmi {

using namespace boost::bimaps;

// Normally vertices are assumed to be in the left, modules in the right.
// What happens if the network is swapped? There is a small chance that
// some branches have to be discarded.
typedef bimap< multiset_of<size_t>, multiset_of<size_t> >
    vertex_module_bimap_t;

typedef vertex_module_bimap_t::relation relation_t;

typedef std::shared_ptr< vertex_module_bimap_t >
    vertex_module_bimap_ptr;

typedef std::pair< vertex_module_bimap_t, vertex_module_bimap_t >
    two_relations_t;

typedef std::shared_ptr< two_relations_t > two_relations_ptr;

typedef std::set< size_t > module_set_t;
//typedef std::set<size_t> remaining_modules_set_t; <-- Now just an alias for
// the same thing.

typedef module_set_t remaining_modules_set_t ;


// I need a couple of bimaps to reflect to full thing structure...

}  // gecmi

#endif // GECMI__VERTEX_MODULE_MAPS_HPP_

