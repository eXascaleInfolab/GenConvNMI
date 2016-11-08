#ifndef GECMI__REPRESENTANTS_HPP_
#define GECMI__REPRESENTANTS_HPP_

#include <unordered_set>
#include <string>
#include <functional>  // hash, function() wrapper
#include <iostream>

#include "bimap_cluster_populator.hpp"


namespace gecmi {

    typedef std::unordered_set< size_t > vertex_set_t;

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
//    	representant_t(representant_t&&) = default;
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

    // Extract the modules for the given vertex.
    void get_modules(
        size_t vertex,
        vertex_module_bimap_t const& vmb1,
        vertex_module_bimap_t const& vmb2,
        // out
        module_set_t& out_mset1,
        module_set_t& out_mset2
        );

    size_t get_vertex_count( vertex_module_bimap_t& vmb );

    using std::ostream;
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

