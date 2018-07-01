#include <iostream>

#include <vector>
#include <unordered_map>
#include "cnl_header_reader.hpp"
#include "cluster_reader.hpp"
#include "vertex_module_maps.hpp"
#include "agghash.hpp"


namespace gecmi {

using std::stoul;
using std::vector;
using std::unordered_map;

// Note: unordered_map<size_t>, where size_t is std::hash may cause omission of distinct clusters having the same hash
using ClusterHash = daoc::AggHash<>;
using ClusterHashes = vector<ClusterHash>;  // The same size_t (ClusterHash::hash) can be yielded for distinct ClusterHash
// Note: unordered_map should have keys of size_t, but distinct AggHash
// may have the same AggHash::hash() : size_t. ClusterHashes are used as
// values to guarantee that all distinct clusters (AggHash) are present even
// if distinct AggHash have the same AggHash::hash() : size_t.
using ClustersHashes = unordered_map<ClusterHash::IdT, ClusterHashes>;


// size_t read_clusters( istream& input, input_interface& ) {{{
size_t read_clusters( istream& input, input_interface& inp_interf, const char* fname,
	IdMap* idmap, float membership, bool fltdups, size_t* nmods)
{
    // Note: CNL [CSN] format only is supported
	string  line;
	size_t  clsnum = 0;  // The number of clusters
	size_t  ndsnum = 0;  // The number of nodes
	parseHeader(input, line, clsnum, ndsnum);

	const size_t  cmsbytes = ndsnum ? 0 : inputSize(input, fname);
	const bool  estimated = !ndsnum || !clsnum
		? estimateSizes(ndsnum, clsnum, cmsbytes, membership) : false;  // Whether the number of nodes/clusters is estimated
//#ifdef DEBUG
//	fprintf(stderr, "> read_clusters(), %lu bytes"
//		" => estimated %lu nodes, %lu clusters\n", cmsbytes, ndsnum, clsnum);
//#endif // DEBUG

	//fprintf(stderr, "> read_clusters(), %lu clusters, %lu nodes\n", clsnum, ndsnum);
	if(clsnum || ndsnum) {
		// Note: reserve more than ndsnum * membership in case membership is not specified and overlaps are present
		const size_t  rsvsize = ndsnum * membership + clsnum;  // Note: bimap has the same size of both sides
#ifdef DEBUG
		fprintf(stderr, "> read_clusters(), preallocating"
			" %lu (%lu, %lu) elements, estimated: %u\n", rsvsize, ndsnum, clsnum, estimated);
#endif // DEBUG
        inp_interf.reserve_vertices_modules(rsvsize, rsvsize);
	}

	// Preallocate hashes for the clusters if required
	ClustersHashes  cshs;
	if(fltdups)
		cshs.reserve(clsnum);

	ClusterHash  chash;
	vertices_t  cmbs;  // Cluster members
	size_t ndupcls = 0;  // The number of omitted duplicated clusters
    size_t iline = 0;  // Payload line index (internal id of the cluster)
    size_t members = 0;  // Evaluate the actual number of members (nodes including repetitions)
	Id  uid = idmap ? idmap->size() : 0;   // Unique id
    // Preallocate idmap initially
    if(idmap && !idmap->size())
		idmap->reserve(ndsnum / sqrt(clsnum));  // Consider overlaps to not over allocate
    do {
        char *tok = strtok(const_cast<char*>(line.data()), " \t");

        // Skip comments
        // Note: Boost bimap of multiset does not support .reserve(),
        // but has rehash()
        // so do not look for the header
        if(!tok || tok[0] == '#')
            continue;
        ++iline;  // Start modules (clusters) id from 1
        // Skip the cluster id if present
        if(tok[strlen(tok) - 1] == '>') {
            tok = strtok(nullptr, " \t");
            // Skip empty clusters
            if(!tok)
                continue;
        }
        do {
        	size_t  id = stoul(tok);  // Allow input ids to have huge range
			// Remap input ids to form a solid range if required
			if(idmap) {
				auto res = idmap->emplace(id, uid);
				if(res.second)
					id = uid++;
				else id = res.first->second;
			}

            // Note: this algorithm does not support fuzzy overlaps (nodes with defined shares),
            // the share part is skipped if exists
			if(fltdups) {
				if(id > std::numeric_limits<ClusterHash::IdT>::max())
					throw std::range_error("Id '" + std::to_string(id)
						+ "' is too large to be used with the ClusterHash");
				chash.add(id);
				cmbs.push_back(id);
			} else {
				inp_interf.add_vertex_module(id, iline);
				// Note: the number of nodes can't be evaluated here simply incrementing the value,
				// because clusters might have overlaps, i.e. the nodes might have multiple membership
				++members;
			}
        } while((tok = strtok(nullptr, " \t")));
        // Retain the unique clusters in the duplicates filtering mode
        if(fltdups) {
			// Add the cluster if such cluster has not been added yet
			const auto ch = chash.hash();
			const auto ich = cshs.find(ch);
			if(ich == cshs.end()
			|| std::find(ich->second.begin(), ich->second.end(), chash) == ich->second.end()) {
				for(auto id: cmbs)
					inp_interf.add_vertex_module(id, iline);
				++members += cmbs.size();
				cshs[ch].push_back(chash);
			} else {
				++ndupcls;
				--iline;  // Decrease clusters id to retain the solid range
			}
			chash.clear();
			cmbs.clear();
		}
    } while(getline(input, line));  // Note: the line does not contain '\n' in the end, EOL is trimmed

	// Rehash the nodes decreasing the allocated space and number of buckets
	// for the faster iterating if required
	inp_interf.shrink_to_fit_modules();

    const size_t  ansnum = inp_interf.uniqlSize();;  // Evaluate the actual number of nodes, resulting value
#ifdef DEBUG
	fprintf(stderr, "> read_clusters(), expected & actual"
		" nodes: %lu -> %lu, clusters: %lu -> %lu; nodes membership: %G\n"
		, ndsnum, ansnum, clsnum, iline, float(members) / ansnum);
#endif // DEBUG
	if(!estimated && ((clsnum && clsnum != iline) || (ndsnum && ndsnum != ansnum)))
		fprintf(stderr, "WARNING read_clusters(),"
			" The specified number of nodes/clusters does not correspond to the actual one"
			"  nodes: %lu -> %lu, clusters: %lu -> %lu\n"
			, ndsnum, ansnum, clsnum, iline);
	if(ndupcls)
		fprintf(stderr, "WARNING read_clusters(), %lu duplicated clusters omitted"
			" from the input file\n", ndupcls);
	// Output the number of loaded UNIQUE modules
	if(nmods)
		*nmods = iline;

	return ansnum;
} // Reader function }}}

}  // gecmi
