#include <iterator>
#include <algorithm>  // set_intersection

#include "representants.hpp"


namespace gecmi{

    // void get_modules( vertex, vmb1, vmb2, o1, o2 ) {{{
    void get_modules(
        size_t vertex,
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2,
        // out
        module_set_t& out_mset1,
        module_set_t& out_mset2
        )
    {
        // A map from vertex ( a single one) to a set of modules

        // Get the modules 1
        auto range = vmb1.left.equal_range(vertex);
        for(auto ivm = range.first; ivm != range.second; ++ivm)
			out_mset1.insert(ivm->second);

        // Get the modules 2
        range = vmb2.left.equal_range(vertex);
        for(auto ivm = range.first; ivm != range.second; ++ivm)
			out_mset2.insert(ivm->second);
    } // }}}

//    // representant_set_t extract_representants( vmb1, vmb2 ) {{{
//    representant2appereances_map_t extract_representants(
//        vertex_module_bimap_t const& vmb1,
//        vertex_module_bimap_t const& vmb2
//    )
//    {
//        representant2appereances_map_t results;
//
//        // A map from vertex ( a single one) to a set of modules
//        auto const& n1_2_modules = vmb1.left;
//        auto const& n2_2_modules = vmb2.left;
//
////        size_t last_vertex = 0;
////        bool is_first = true;
//        // Get vertex set.
//        vertex_set_t vertices;
//        for( auto const& r: vmb1 )
//            vertices.insert( r.left );
//        //
//
//        // Now go
//        for( size_t vertex: vertices )
//        {
//            //std::cout <<"vertex " << vertex << std::endl;
//            // Get the modules 1
//            modules_set_t ms1, ms2;
//            for( auto t=n1_2_modules.lower_bound( vertex ),
//                 t_end =n1_2_modules.upper_bound( vertex );
//                 t != t_end; t ++ )
//            {
//                ms1.insert( t->second );
//                //std::cout <<"m1 " << t->second << std::endl;
//            }
//
//            // Get the modules 2
//            for( auto t=n2_2_modules.lower_bound( vertex ),
//                 t_end =n2_2_modules.upper_bound( vertex );
//                 t != t_end; t ++ )
//            {
//                ms2.insert( t->second );
//                //std::cout <<"m2 " << t->second << std::endl;
//            }
//
//            representant_t rep( ms1, ms2 );
//            results[ rep ] += 1;
////            representant_fw_t rep_fw ( rep );
////            if ( results.count( rep_fw ) )
////            {
////                results[ rep_fw ] += 1;
////            } else
////            {
////                results[ rep_fw ] = 1;
////            }
//            //results.insert( rep_fw );
//        }
//
//        //std::cout << "Representant count: " << results.size() << std::endl;
//
//        return results;
//    } // }}}

    // size_t get_vertex_count( vertex_module_bimap_t const& vmb ) {{{
    size_t get_vertex_count( vertex_module_bimap_t& vmb )
    {
    	return uniqSize(vmb.left);
    } // }}}

//    // struct representant_tracker::pimpl_t {{{
//    class representant_tracker::pimpl_t {
//        representant_value_set_t  seen_representants;
//        //representant_values_t  representants;  // Represents store
//        modules_set_t original_1, original_2;
//
//	public:
//        pimpl_t( modules_set_t const& original_1,
//                 modules_set_t const& original_2 )
//		: seen_representants(), original_1( original_1 ), original_2( original_2 )
//        {
//            seen_representants.emplace(original_1, original_2);
//        }
//
//        representant_t build_representant( modules_set_t const& mset1, modules_set_t const& mset2 )
//        {
//            module_set_t i_ms1, i_ms2;
//            std::set_intersection(mset1.begin(), mset1.end(), original_1.begin()
//				, original_1.end(), std::inserter(i_ms1, i_ms1.begin()));
//            std::set_intersection(mset2.begin(), mset2.end(), original_2.begin()
//				, original_2.end(), std::inserter(i_ms2, i_ms2.begin()));
//
//            return representant_t( i_ms1, i_ms2 );
//        }
//
//        bool set( modules_set_t const& mset1, modules_set_t const& mset2)
//        {
//            return seen_representants.insert(
//                build_representant(mset1, mset2) ).second;
//        }
//
//        bool is_set( modules_set_t const& mset1, modules_set_t const& mset2)
//        {
//            return seen_representants.count(
//                build_representant( mset1, mset2 ) );
//        }
//    }; // }}}


//	representant_tracker::representant_tracker(
//		modules_set_t const& original_1,
//		modules_set_t const& original_2):
//		impl( new pimpl_t( original_1, original_2 ) )
//	{}
//
//	// Returns 'true' if a new element is inserted
//	bool representant_tracker::set( modules_set_t const& mset1, modules_set_t const& mset2 )
//	{
//		return impl->set( mset1, mset2);
//	}
//
//	// Returns 'true' if  the element is set
//	bool representant_tracker::is_set( modules_set_t const& mset1, modules_set_t const& mset2 )
//	{
//		return impl->is_set( mset1, mset2 );
//	}
//
//	representant_tracker::~representant_tracker()
//	{
//	   delete impl;
//	}

}  // gecmi
