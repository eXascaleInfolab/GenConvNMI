#ifndef GECMI__REPRESENTANTS_HPP_
#define GECMI__REPRESENTANTS_HPP_

#include <set>
#include <map>
#include <utility>
#include <iostream>

#include <boost/flyweight.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include "bimap_cluster_populator.hpp"


namespace gecmi {

    typedef std::set< size_t > modules_set_t;
    typedef std::set< size_t > vertex_set_t;

    typedef std::pair< modules_set_t, modules_set_t >
        representant_t;

    // A standard implementation of representant tracking. As of now,
    // this one does intersection only with the original sets.
    class representant_tracker {
        struct pimpl_t;
        pimpl_t* impl;
    public:
        representant_tracker(
            modules_set_t const& original_1,
            modules_set_t const& original_2);
        representant_tracker(representant_tracker const&) = delete;

        representant_tracker& operator=(const representant_tracker&) = delete;

        // Returns 'true' if a new element is inserted
        bool set( modules_set_t const& mset1, modules_set_t const& mset2 );
        // Returns 'true' if  the element is set, that is, if the representant
        // of this module set has been already used.
        bool is_set( modules_set_t const& mset1, modules_set_t const& mset2 );

        ~representant_tracker();
    };

    typedef boost::flyweights::flyweight< representant_t >
        representant_fw_t;

    typedef boost::optional< representant_fw_t >
        representant_ofw_t;

    typedef std::set< representant_t >
        representant_value_set_t;

    typedef std::set< representant_fw_t >
        representant_set_t;

    typedef std::map< representant_fw_t, size_t >
        representant2appereances_map_t;

    // Extract the modules for the given vertex.
    void get_modules(
        size_t vertex,
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2,
        // out
        module_set_t& out_mset1,
        module_set_t& out_mset2
        );

    representant2appereances_map_t extract_representants(
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2
        );

    size_t get_vertex_count( vertex_module_bimap_t const& vmb );

    using std::ostream;
    inline ostream& operator<<( ostream& out, representant_t const& rep )
    {
        out << "( ";
        //for ( size_t i: rep.first ) out << i << " " ;
        BOOST_FOREACH( size_t i, rep.first )
        {
            out << i << " " ;
        }
        out << " | " ;
        //for ( size_t i: rep.second ) out << i << " " ;
        BOOST_FOREACH( size_t i, rep.second )
        {
            out << i << " " ;
        }
        out << " ) " ;
        return out;
    }

    inline ostream& operator<<( ostream& out, module_set_t const& mset )
    {
        out << "(";
        //for ( size_t i: mset ) out << i << " " ;
        BOOST_FOREACH( size_t i, mset )
        {
            out << i << " " ;
        }
        out << " )";
        return out;
    }

    inline ostream& operator<<( ostream& out, representant_ofw_t const& rep )
    {
        if ( not rep )
        {
            out << "<<null>>" ;
        } else
        {
            representant_fw_t rep1 = *rep;
            out << rep1.get() ;
        }
        return out;
    }

    inline ostream& operator<<( ostream& out, representant_set_t const& rep_set )
    {
        out << "{ " ;
        BOOST_FOREACH( representant_t const& rep, rep_set )
        {
            out << rep << " " ;
        }
        out << " }" ;
        return out;
    }

}  // gecmi

#endif // GECMI__REPRESENTANTS_HPP_

