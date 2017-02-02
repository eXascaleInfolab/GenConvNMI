#include <iostream>
#include <sstream>
#include <string>
#include <cstring>  // strtok
#include <cmath>  // sqrt

#ifdef __unix__
#include <sys/stat.h>
#endif // __unix__

#include "cluster_reader.hpp"


namespace gecmi {

using std::string;
using std::stoul;
using std::istream;
using std::istringstream;


template <typename NodeId>
void parseHeader(istream& fsm, string& line, size_t& clsnum, size_t& ndsnum) {
	// Process the header, which is a special initial comment
	// The target header is:  # Clusters: <cls_num>[,] Nodes: <cls_num>
	const string  clsmark = "clusters";
	const string  ndsmark = "nodes";
	while(getline(fsm, line)) {
		// Skip empty lines
		if(line.empty())
			continue;
		// Consider only subsequent comments
		if(line[0] != '#')
			break;

		// 1. Replace the staring comment mark '#' with space to allow "#clusters:"
		// 2. Replace ':' with space to allow "Clusters:<clsnum>"
		for(size_t pos = 0; pos != string::npos; pos = line.find(':', pos + 1))
			line[pos] = ' ';

		istringstream fields(line);
		string field;

		// Read clusters specification
		fields >> field;
		if(field.length() != clsmark.length())
			continue;
		// Convert to lower case
		for(size_t i = 0; i < field.length(); ++i)
			field[i] = tolower(field[i]);
		if(field != clsmark)
			continue;
		fields >> clsnum;

		// Read nodes specification
		fields >> field;
		// Allow optional ','
		if(!field.empty() && field[0] == ',')
			fields >> field;
		if(field.length() == ndsmark.length()) {
			for(size_t i = 0; i < field.length(); ++i)
				field[i] = tolower(field[i]);
			if(field == ndsmark)
				fields >> ndsnum;
		}
		// Get following line for the unified subsequent processing
		getline(fsm, line);
		break;
	}
}

void estimateSizes(size_t cmsbytes, size_t& ndsnum, size_t& clsnum)
{
	if(!cmsbytes)
		return;

	size_t  magn = 10;  // Decimal ids magnitude
	unsigned  img = 1;  // Index of the magnitude (10^1)
	size_t  reminder = cmsbytes % magn;  // Reminder in bytes
	ndsnum = reminder / ++img;  //  img digits + 1 delimiter for each element
	while(cmsbytes >= magn) {
		magn *= 10;
		ndsnum += (cmsbytes - reminder) % magn / ++img;
		reminder = cmsbytes % magn;
	}

	// Usually the number of clusters does not increase square root of the number of nodes
	clsnum = sqrt(ndsnum) + 1;  // Note: +1 to consider rounding down
}

// void read_clusters_without_remappings( istream& input, input_interface& ) {{{
void read_clusters_without_remappings( istream& input,
    input_interface& inp_interf, const char* fname )
{
    // Note: CNL [CSN] format only is supported
	string  line;
	size_t  clsnum = 0;  // The number of clusters
	size_t  ndsnum = 0;  // The number of nodes
	parseHeader<size_t>(input, line, clsnum, ndsnum);

	if(!ndsnum) {
		size_t  cmsbytes = 0;
#ifdef __unix__
		if(fname) {
			struct stat  filest;
			if(!stat(fname, &filest))
				cmsbytes = filest.st_size;
		}
		//fprintf(stderr, "# %s: %lu bytes\n", fname, cmsbytes);
#endif // __unix
		// Get length of the file
		if(!cmsbytes) {
			input.seekg(0, input.end);
			cmsbytes = input.tellg();  // The number of bytes in the input communities
			input.seekg(0, input.beg);
		}
		if(cmsbytes && cmsbytes != size_t(-1))  // File length fetching failed
			estimateSizes(cmsbytes, ndsnum, clsnum);
	}

	//fprintf(stderr, "# %lu clusters, %lu nodes\n", clsnum, ndsnum);
	if(clsnum || ndsnum) {
		const size_t  rsvsize = ndsnum + clsnum;  // Note: bimap has the same size of both sides
        inp_interf.reserve_vertices_modules(rsvsize, rsvsize);
	}

    size_t iline = 0;  // Payload line index (internal id of the cluster)
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
        } while((tok = strtok(nullptr, " \t")));
    } while(getline(input, line));

	// Rehash the nodes decreasing the allocated space and number of buckets
	// for the faster iterating if required
	inp_interf.shrink_to_fit_modules();
} // Reader function }}}

}  // gecmi
