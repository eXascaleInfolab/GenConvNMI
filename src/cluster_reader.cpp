#include <iostream>
#include <string>
#include <cstring>  // strtok

#include "cluster_reader.hpp"


namespace gecmi {

using std::string;
using std::stoul;


// void read_clusters_without_remappings( istream& input, input_interface& ) {{{
void read_clusters_without_remappings(
    std::istream& input,
    input_interface& inp_interf )
{
    // Note: CNL [CSN] format only is supported
    string line;

    size_t iline = 0;
    while(getline(input, line)) {
        char *tok = strtok(const_cast<char*>(line.data()), " \t");

        // Skip comments
        // Note: Boost bimap of multiset does not support .reserve(),
        // so do not look for the header
        if(!tok || tok[0] == '#')
            continue;
        ++iline;  // Start modules (clusters) id from 1
        do {
            inp_interf.add_vertex_module(stoul(tok), iline);
        } while((tok = strtok(nullptr, " \t")));
    }
} // Reader function }}}

}  // gecmi
