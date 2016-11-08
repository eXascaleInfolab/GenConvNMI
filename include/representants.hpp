#ifndef GECMI__REPRESENTANTS_HPP_
#define GECMI__REPRESENTANTS_HPP_

//#include <list>
#include <unordered_set>
//#include <unordered_map>
//#include <utility>
#include <string>
#include <functional>  // hash, function() wrapper
#include <iostream>

//#include <boost/flyweight.hpp>
//#include <boost/optional.hpp>
//#include <boost/functional/hash.hpp>

#include "bimap_cluster_populator.hpp"


namespace gecmi {

    typedef std::unordered_set< size_t > vertex_set_t;

//    typedef std::pair< modules_set_t, modules_set_t >  representant_t;

    class representant_t {
//    	modules_set_t first;
//    	modules_set_t second;

    	constexpr static size_t  hashlen = 3*2;  // In items (size_t): num, sum, sum2
    	size_t  hash[hashlen];
    	size_t  key;  // Required if used in the unordered containers
	public:
    	representant_t(const modules_set_t& s1, const modules_set_t& s2)
    	//: first(s1), second(s2)
    	: key(0)
    	{
    		hash[0] = s1.size();
    		for(auto v: s1) {
				hash[2] += v;
				hash[4] += v*v;
    		}

    		hash[1] = s2.size();
    		for(auto v: s2) {
				hash[3] += v;
				hash[5] += v*v;
    		}

    		//key = boost::hash_value(hash);
			key = std::hash<std::string>()(std::string(
				reinterpret_cast<const char*>(hash), sizeof hash));
    	}

    	representant_t() = default;
    	representant_t(representant_t&&) = default;
//    	representant_t(const representant_t&) = default;

    	bool operator ==(const representant_t& r) const  { return key == r.key; }
//    	{
//    		//return !memcmp(hash, r.hash, sizeof hash);
//    		for(size_t i = 0; i < hashlen; ++i)
//				if(hash[i] != r.hash[i])
//					return false;
//			return true;
//		}

		// Hasher for the STL containers
		static size_t hashfn(const representant_t& r)  { return r.key; }
    };

	// Note only const object of this type should be used
    //typedef const _representant_t  representant_t;

//    struct ReprHasher {
//    	size_t operator()(const representant_t& r) const  { return _representant_t::hashfn(r); }
//    };


//    // A standard implementation of representant tracking. As of now,
//    // this one does intersection only with the original sets.
//    class representant_tracker {
//        struct pimpl_t;
//        pimpl_t* impl;
//    public:
//        representant_tracker(
//            modules_set_t const& original_1,
//            modules_set_t const& original_2);
//        representant_tracker(representant_tracker const&) = delete;
//
//        representant_tracker& operator=(const representant_tracker&) = delete;
//
//        // Returns 'true' if a new element is inserted
//        bool set( modules_set_t const& mset1, modules_set_t const& mset2 );
//        // Returns 'true' if  the element is set, that is, if the representant
//        // of this module set has been already used.
//        bool is_set( modules_set_t const& mset1, modules_set_t const& mset2 );
//
//        ~representant_tracker();
//    };

//	typedef representant_t  representant_fw_t;
//
//	// Note: no_tracking is used because anyway the items are not deleted
//    typedef boost::flyweights::flyweight< representant_t , no_tracking >
//        representant_fw_t;+
//
//    typedef boost::optional< representant_fw_t >
//        representant_ofw_t;


//    typedef std::list< representant_t >
//        representant_values_t;
//
//    typedef std::unordered_set< representant_t, std::function<decltype(representant_t::hashfn)> >
//        representant_value_set_t;
//
//    typedef std::unordered_set< representant_fw_t >
//        representant_set_t;
//
//    typedef std::unordered_map< representant_t*, size_t >  // representant_fw_t
//        representant2appereances_map_t;

    // Extract the modules for the given vertex.
    void get_modules(
        size_t vertex,
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2,
        // out
        module_set_t& out_mset1,
        module_set_t& out_mset2
        );

//    representant2appereances_map_t extract_representants(
//        vertex_module_bimap_t const& vmb1,
//        vertex_module_bimap_t const& vmb2
//        );

    size_t get_vertex_count( vertex_module_bimap_t& vmb );

    using std::ostream;
//    inline ostream& operator<<( ostream& out, representant_t const& rep )
//    {
//        out << "( ";
//        //for ( size_t i: rep.first ) out << i << " " ;
//        for( size_t i: rep.first )
//        {
//            out << i << " " ;
//        }
//        out << " | " ;
//        //for ( size_t i: rep.second ) out << i << " " ;
//        for( size_t i: rep.second )
//        {
//            out << i << " " ;
//        }
//        out << " ) " ;
//        return out;
//    }

    inline ostream& operator<<( ostream& out, module_set_t const& mset )
    {
        out << "(";
        //for ( size_t i: mset ) out << i << " " ;
        for( size_t i: mset )
            out << i << " " ;
        out << " )";
        return out;
    }

//    inline ostream& operator<<( ostream& out, representant_ofw_t const& rep )
//    {
//        if ( not rep )
//        {
//            out << "<<null>>" ;
//        } else
//        {
//            representant_fw_t rep1 = *rep;
//            out << rep1.get() ;
//        }
//        return out;
//    }
//
//    inline ostream& operator<<( ostream& out, representant_set_t const& rep_set )
//    {
//        out << "{ " ;
//        for( representant_t const& rep: rep_set )
//        {
//            out << rep << " " ;
//        }
//        out << " }" ;
//        return out;
//    }

}  // gecmi

#endif // GECMI__REPRESENTANTS_HPP_

