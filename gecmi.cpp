#include <fstream>
#include <stdexcept>
#include <system_error>

#include <boost/program_options.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "cluster_reader.hpp"
#include "bimap_cluster_populator.hpp"
#include "calculate_till_tolerance.hpp"

using std::string;
using std::vector;
using std::cout;
using std::ifstream;
using std::to_string;
using std::invalid_argument;
using std::domain_error;
using std::system_error;
namespace po = boost::program_options;
using namespace gecmi;


int main(int argc, char* argv[])
{
    string  descrstr = string("Generalized Conventional Mutual Information (GenConvMI)\n"
        "Evaluates NMI (normalized by max and sqrt) for overlapping (soft, fuzzy) clusters (communities)"
        ", compatible with standard NMI\n"
        "https://github.com/eXascaleInfolab/GenConvNMI"
        "\n\nUsage:\t").append(argv[0]).append(" [options] <clusters1> <clusters2>\n"
        "clusters  - clusters file in the CNL format (https://github.com/eXascaleInfolab/PyCABeM/blob/master/formats/format.cnl),"
        " where each line lists space separated ids of the cluster members\n"
        "\nOptions");

    po::options_description desc(descrstr);
    po::positional_options_description p;
    p.add( "input", 2 );
    desc.add_options()
        ("help,h", "produce help message")
        ("input",
            po::value<vector<string>>()->composing(),
            "name of the input files" )
        ("sync,s",
            po::value<string>(),
            "synchronize the node base omitting the non-matching nodes\n"
            "NOTE: The node base is either the first input file or '-' (automatic selection"
            " of the input file having the least number of nodes)")
        ("id-remap,i", "remap ids allowing arbitrary input ids (non-contiguous ranges)"
            ", otherwise ids should form a solid range and start from 0 or 1")
        ("nmis,n", "output both NMI [max] and NMI_sqrt")
        ("fnmi,f", "evaluate also FNMI, includes '-n'")
        ("risk,r",
            po::value<double>()->default_value(0.01),
            "probability of value being outside" )
        ("error,e",
            po::value<double>()->default_value(0.01),
            "admissible error" )
        ("fast,a", "apply fast approximate evaluations that are less accurate"
            ", but much faster on large networks")
        ("membership,m",
            po::value<float>()->default_value(1.f),
            "average expected membership of nodes in the clusters, > 0, typically >= 1")
        ("retain-dups,d", "retain duplicated clusters if any instead of filtering them out"
            " (not recommended)")
    ;
    po::variables_map vm;
    po::store( po::command_line_parser(argc, argv)
       .options(desc)
       .positional(p).run(), vm
    );
    po::notify(vm);
    if ( vm.count("help" ) )
    {
        cout << desc << std::endl;
        return 1;
    }
    vector< string > positionals;
    // Whether the node base is explicitly specified as the first input file (not "-")
    const bool  ndbase1 = vm.count("sync") && vm["sync"].as<string>().compare("-");
    try {
        // Consider that the first input file can be a sync node base
        if(ndbase1)
            positionals.push_back(vm["sync"].as<string>());
        for(auto& finp: vm["input"].as<vector<string>>())
            positionals.push_back(finp);
    } catch ( boost::bad_any_cast const& ) {
        fprintf(stderr, "Please, provide two input files to proceed. Use `gecmi -h` for more info\n");
        throw;
    }
    if ( positionals.size() != 2 )
        throw invalid_argument("Please provide exactly two input files as input\n");

    ifstream in1(positionals[0].c_str());
    if( !in1 )
        throw system_error(errno, std::system_category(), "Could not open the first file\n");

    ifstream in2(positionals[1].c_str());
    if( !in2 )
        throw system_error(errno, std::system_category(), "Could not open the second file\n");


    // Read the clusters
    two_relations_t  two_rel;
    bimap_cluster_populator  bcp1( two_rel.first );
    bimap_cluster_populator  bcp2( two_rel.second );
    // bimap_cluster_populator:  left: Nodes, right: Clusters
    size_t  b1lnum=0, b2lnum=0;  // The number of nodes in the collections
    const float membership = vm["membership"].as<float>();

    if(membership <= 0)
        throw invalid_argument("membership = " + to_string(membership)
			+ " should be positive");

    {
        const bool remap = vm.count("id-remap");  // Remap ids
        const bool fltdups = !vm.count("retain-dups");  // Filter out duplicated clusters
        IdMap idmap;  // Mapping of ids to provide solid range starting from 0 if required
        size_t  nmods1 = 0;  // The number of UNIQUE clusters (modules) in the first collection
        size_t  nmods2 = 0;

#ifdef DEBUG
        fprintf(stderr, "Loading %s...\n", positionals[0].c_str());
#endif  // DEBUG
        b1lnum = read_clusters(
            in1,
            bcp1,
            positionals[0].c_str(),
            remap ? &idmap : nullptr,
            membership, fltdups, &nmods1
        );

#ifdef DEBUG
        fprintf(stderr, "Loading %s...\n", positionals[1].c_str());
#endif  // DEBUG
        b2lnum = read_clusters(
            in2,
            bcp2,
            positionals[1].c_str(),
            remap ? &idmap : nullptr,
            membership, fltdups, &nmods2
        );

        // Consider the case of single cluster collections, where NMI is not applicable
        if(nmods1 != nmods2 && (nmods1 == 1 || nmods2 == 1))
            throw domain_error("ERROR, NMI is not applicable for the single cluster collections\n");
    }
#ifdef DEBUG
        assert(b1lnum == bcp1.uniqlSize() && b2lnum == bcp2.uniqlSize()
            && "");
#endif // DEBUG

    if(b1lnum != b2lnum) {
        const bool  sync = vm.count("sync");
        fprintf(stderr, "WARNING, evaluating collections have different number of nodes: %lu != %lu"
            ", sync enabled: %s (forced to finp1: %s)\n", b1lnum, b2lnum
            , sync ? "yes" : "no (the quality will be penalized)", ndbase1 ? "yes" : "no");

        // Synchronize the number of nodes in both collections if required
        if (sync) {
            // Sync to bcp1 if the sync is automatic ("-") and bcp1 has the lowest number of nodes
            // or if the sync is forced to bcp1 (not "-")
            if(b1lnum <= b2lnum || ndbase1) {  // bcp1 is the base for the sync, the nodes are removed from bcp2
                bcp2.sync(bcp1);
                b2lnum = bcp2.uniqlSize();
            } else {
                bcp1.sync(bcp2);  // bcp2 is the base for the sync, the nodes are removed from bcp1
                b1lnum = bcp1.uniqlSize();
            }
            // Show WARNING if the synchronization is failed or
            if(b1lnum != b2lnum) {
                //throw domain_error("Input collections have different node base and can't be synchronized gracefully: "
                //    + to_string(b1lnum) + " != " + to_string(b2lnum)+ "\n");
                fprintf(stderr, "WARNING, full synchronization failed, the nodes in the collections differ"
                    " after the partial synchronization: %lu != %lu\n", b1lnum, b2lnum);
            }
        }
    }

    const double risk = vm["risk" ].as<double>();
    const double epvar = vm["error"].as<double>();

    calculated_info_t cit = calculate_till_tolerance( two_rel, risk, epvar
        , vm.count("fast"), b1lnum, b2lnum );

    if (vm.count("fnmi")) {
        const auto b1rnum = bcp1.uniqrSize();
        const auto b2rnum = bcp2.uniqrSize();
        printf("NMI_max: %G, FNMI: %G, NMI_sqrt: %G; cls1: %lu, cls2: %lu\n", cit.nmi
              // Note: 2^x is used instead of e^x to have the same base as in the log
            , cit.nmi * pow(2, -double(abs(b1rnum - b2rnum)) / std::max(b1rnum, b2rnum))
            , cit.nmi_sqrt, b1rnum, b2rnum);
    } else if (vm.count("nmis"))
        printf("NMI_max: %G, NMI_sqrt: %G\n", cit.nmi, cit.nmi_sqrt);
    else printf("%G\n", cit.nmi);

    return 0;
}
