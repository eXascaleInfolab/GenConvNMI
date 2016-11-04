#ifndef  GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_
#define  GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_

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
        const_cast<typename MapT::iterator&>(ind) = mc.upper_bound(ind->first);
        ++num;
    }

    return num;
}

// class bimap_cluster_populator {{{
class bimap_cluster_populator:
    public mock_input_processor
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

    virtual ~bimap_cluster_populator() = default;

    virtual void add_vertex_module( int internal_vertex_id, int module_id )
    {
        vmb.insert( relation_t( internal_vertex_id, module_id ) );
    };

    size_t uniqlSize() const  { return uniqSize(vmb.left); }

    size_t uniqrSize() const  { return uniqSize(vmb.right); }

//    bool isSynced(const bimap_cluster_populator& bcpbase) const
//    {
//        return bcpbase.uniqlSize() == uniqlSize();  // The number of nodes is the same (the same node base)
//    }

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
                    = vmb.left.erase(ind, vmb.left.upper_bound(ind->first));
            else ++const_cast<typename decltype(vmb.left)::iterator&>(ind);
        }
    }
}; // class bimap_cluster_populator }}}

}  // gecmi

#endif // GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_
