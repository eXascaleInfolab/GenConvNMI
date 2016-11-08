#ifndef  GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_
#define  GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_

#include <type_traits>
#include <cassert>

#include "vertex_module_maps.hpp"
#include "cluster_reader.hpp"


namespace gecmi {

class bimap_cluster_populator;
void sync(bimap_cluster_populator& bcp1, bimap_cluster_populator& bcp2);
double fairRatio(const bimap_cluster_populator& bcp1, const bimap_cluster_populator& bcp2);

template<typename MapT>
size_t uniqSize(MapT& mc)
{
    size_t  num = 0;
    for(const auto& ind = mc.begin(); ind != mc.end();) {
        const_cast<typename std::remove_const_t<MapT>::iterator&>(ind)
            = mc.equal_range(ind->first).second;
        ++num;
    }

    return num;
}

class bimap_cluster_populator: public input_interface
{
    friend void sync(bimap_cluster_populator& bcp1, bimap_cluster_populator& bcp2);
    friend double fairRatio(const bimap_cluster_populator& bcp1, const bimap_cluster_populator& bcp2);

    vertex_module_bimap_t& vmb;
    // left: Nodes, right: Clusters
public:
    typedef vertex_module_bimap_t::relation relation_t;
    bimap_cluster_populator( vertex_module_bimap_t& vmb ):
        vmb(vmb)
    {}

    void add_vertex_module( size_t internal_vertex_id, size_t module_id )
    {
        vmb.insert( typename vertex_module_bimap_t::value_type(internal_vertex_id, module_id) );
    };

    void reserve_vertices_modules( size_t vertices_num, size_t modules_num )
    {
        // Note: boost bimap of multiset does not have reserve()
        //vmb.left.reserve(vertices_num);
        //vmb.right.reserve(modules_num);
    };

    size_t uniqlSize() const  { return uniqSize(vmb.left); }

    size_t uniqrSize() const  { return uniqSize(vmb.right); }

    //! \brief Synchronize nodes with the base collection
    //! \pre nodesize > bbcpbase nodes size
    //!
    //! \param bcpbase const bimap_cluster_populator&  - the base collection,  objective of the synchronization
    //! \return void
    void sync(const bimap_cluster_populator& bcpbase)
    {
#ifdef DEBUG
        if(bcpbase.uniqlSize() >= uniqlSize()) {
            assert(0 && "Base collection should be always smaller than the refining one");
            return;
        }
#endif // DEBUG

        // Remove all the nodes with their relations that are absent in the base collection
        for(const auto& ind = vmb.left.begin(); ind != vmb.left.end();) {
            // Remove the absent node with all relations
            if(bcpbase.vmb.left.find(ind->first) == bcpbase.vmb.left.end())
                //auto indn = vmb.left.erase(ind, vmb.left.upper_bound(ind->first));
                const_cast<typename decltype(vmb.left)::iterator&>(ind)
                    = vmb.left.erase(ind, vmb.left.equal_range(ind->first).second);
            else ++const_cast<typename decltype(vmb.left)::iterator&>(ind);
        }
    }
}; // bimap_cluster_populator

}  // gecmi

#endif // GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_
