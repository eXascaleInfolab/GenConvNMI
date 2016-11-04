#include <iostream>
#include <string>
#include <cstring>

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
        if(!tok || tok[0] == '#')
            continue;
        ++iline;
        do {
            inp_interf.add_vertex_module(stoul(tok), iline);
        } while(tok = strtok(nullptr, " \t"));
    }
} // Reader function }}}

}  // gecmi
