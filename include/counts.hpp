#ifndef GECMI__COUNTS_HPP_
#define GECMI__COUNTS_HPP_

#include "bimap_cluster_populator.hpp"


namespace gecmi {

    // So, we can use this as a size.
    inline size_t highest_module_plus_one( vertex_module_bimap_t const& vmb )
    {
        size_t result = 0;
        for( auto const& r: vmb )
        {
            if ( r.right > result ) result = r.right;
        }
        return result+1;
    }

    // For getting the combinatorial size of the space, we need the actual
    // number of entities on game (any dictionary).
    inline size_t get_entity_count( vertex_module_bimap_t const& vmb )
    {
        size_t result = 0;
        int seeing = -1;
        for( auto const& r: vmb.left )
        {
            if ( int( r.first ) > seeing )
            {
                seeing = int( r.first );
                result ++;
            }
        }
        return result;
    }

}  //  gecmi

#endif // GECMI__COUNTS_HPP_

