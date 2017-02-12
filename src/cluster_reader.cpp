#include <iostream>

#include "cnl_header_reader.hpp"
#include "cluster_reader.hpp"


namespace gecmi {

using std::stoul;


// size_t read_clusters_without_remappings( istream& input, input_interface& ) {{{
size_t read_clusters_without_remappings( istream& input,
    input_interface& inp_interf, const char* fname, float membership)
{
    // Note: CNL [CSN] format only is supported
	string  line;
	size_t  clsnum = 0;  // The number of clusters
	size_t  ndsnum = 0;  // The number of nodes
	parseHeader(input, line, clsnum, ndsnum);

	const size_t  cmsbytes = ndsnum ? 0 : inputSize(input, fname);
	const bool  estimated = !ndsnum || !clsnum ? estimateSizes(ndsnum, clsnum, cmsbytes, membership) : false;  // Whether the number of nodes/clusters is estimated
//#ifdef DEBUG
//	fprintf(stderr, "# read_clusters_without_remappings(), %lu bytes"
//		" => estimated %lu nodes, %lu clusters\n", cmsbytes, ndsnum, clsnum);
//#endif // DEBUG

	//fprintf(stderr, "# %lu clusters, %lu nodes\n", clsnum, ndsnum);
	if(clsnum || ndsnum) {
		// Note: reserve more than ndsnum * membership in case membership is not specified and overlaps are present
		const size_t  rsvsize = ndsnum * membership + clsnum;  // Note: bimap has the same size of both sides
#ifdef DEBUG
		fprintf(stderr, "# read_clusters_without_remappings(), preallocating"
			" %lu (%lu, %lu) elements, estimated: %u\n", rsvsize, ndsnum, clsnum, estimated);
#endif // DEBUG
        inp_interf.reserve_vertices_modules(rsvsize, rsvsize);
	}

    size_t iline = 0;  // Payload line index (internal id of the cluster)
    size_t members = 0;  // Evaluate the actual number of members (nodes including repetitions)
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
            // Note: this algorithm does not support fuzzy overlaps (nodes with defined shares),
            // the share part is skipped if exists
            inp_interf.add_vertex_module(stoul(tok), iline);
            // Note: the number of nodes can't be evaluated here simply incrementing the value,
            // because clusters might have overlaps, i.e. the nodes might have multiple membership
            ++members;
        } while((tok = strtok(nullptr, " \t")));
    } while(getline(input, line));  // Note: the line does not contain '\n' in the end, EOL is trimmed

	// Rehash the nodes decreasing the allocated space and number of buckets
	// for the faster iterating if required
	inp_interf.shrink_to_fit_modules();

    const size_t  ansnum = inp_interf.uniqlSize();;  // Evaluate the actual number of nodes, resulting value
#ifdef DEBUG
	fprintf(stderr, "# read_clusters_without_remappings(), expected & actual"
		" nodes: (%lu, %lu), clusters: (%lu, %lu); nodes membership: %G\n"
		, ndsnum, ansnum, clsnum, iline, float(members) / ansnum);
#endif // DEBUG
	if(!estimated && ((clsnum && clsnum != iline) || (ndsnum && ndsnum != ansnum)))
		fprintf(stderr, "WARNING read_clusters_without_remappings(),"
			" The specified number of nodes/clusters does not correspond to the actual one"
			"  nodes: (%lu, %lu), clusters: (%lu, %lu)\n"
			, ndsnum, ansnum, clsnum, iline);

	return ansnum;
} // Reader function }}}

}  // gecmi
