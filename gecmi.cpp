#include <map>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "cluster_reader.hpp"
#include "bimap_cluster_populator.hpp"
#include "calculate_till_tolerance.hpp"

namespace po = boost::program_options;


int main( int argc, char* argv[])
{
    using std::string;
    using std::cout;
    using std::cerr;
    using std::endl;
    using namespace gecmi;

    po::options_description desc(
        "Invoke as : \n"
        "    gecmi <options> <clu-file-1> <clu-file-2> \n"
        "\n"
        "Allowed options: \n"
        );

    po::positional_options_description p;
    p.add( "input", 2 );
    desc.add_options()
        ("help,h", "produce help message")
        ("input",
            po::value<std::vector<string> >()->composing(),
            "name of the input files" )
        ("risk,r",
            po::value<double>()->default_value(0.01),
            "probability of value being outside" )
        ("error,e",
            po::value<double>()->default_value(0.01),
            "admissible error" )
    ;
    po::variables_map vm;
    po::store( po::command_line_parser( argc, argv)
       .options(desc)
       .positional(p).run(), vm
    );
    po::notify(vm);
    if ( vm.count("help" ) )
    {
        cout << desc << endl;
        exit( 1 );
    }
    std::vector< std::string > positionals;
    try {
        positionals = vm["input"].as<
            std::vector< std::string > >();
    } catch ( boost::bad_any_cast const& )
    {
        cerr << "Please, provide two input files to proceed. Use `gecmi -h` for more info" << endl;
        exit(2);
    }
    if ( positionals.size() != 2 )
    {
        cerr << "Please provide exactly two input files as input" << endl;
        exit(2);
    }


    std::ifstream in1(positionals[0].c_str() );
    std::ifstream in2(positionals[1].c_str() );
    if( not in1 )
    {
        cerr << "Could not open file " << positionals[0] <<std::endl;
        exit(2);
    };
    if( not in2 )
    {
        cerr << "Could not open file " << positionals[1] <<std::endl;
        exit(2);
    };

    two_relations_ptr two_rel( new two_relations_t );
    bimap_cluster_populator  bcp1( two_rel->first );
    bimap_cluster_populator  bcp2( two_rel->second );

    read_clusters_without_remappings(
        in1,
        bcp1
    );

    read_clusters_without_remappings(
        in2,
        bcp2
    );

    double risk = vm["risk" ].as< double>();
    double epvar  = vm["error"].as<double>();

    calculated_info_t cit = calculate_till_tolerance(
        two_rel, risk, epvar );

    cout << //"NMI: " <<
		cit.nmi << endl;


    return 0;
}
