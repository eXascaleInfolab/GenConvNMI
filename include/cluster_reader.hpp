#ifndef CLUSTER_READER__CLUSTER_READER_HPP_
#define CLUSTER_READER__CLUSTER_READER_HPP_

#include <memory>


namespace gecmi {

// Input interface...
class input_interface {
public:
    virtual void add_vertex_mapping( int internal_vertex_id, int internal_module_id) = 0;
    virtual void add_module_mapping( int internal_vertex_id, int internal_module_id) = 0;
    virtual void add_vertex_module( int internal_vertex_id, int module_id ) = 0;
public:
    virtual ~input_interface() = default;
};

// A test mock
class mock_input_processor : public input_interface {
    virtual void add_vertex_mapping( int internal_vertex_id, int internal_module_id)
    {};
    virtual void add_module_mapping( int internal_vertex_id, int internal_module_id)
    {};
    virtual void add_vertex_module( int internal_vertex_id, int module_id )
    {};
public:
    virtual ~mock_input_processor() = default;
};

typedef std::shared_ptr< input_interface >
    input_interface_ptr;

// The name of this method reflects the fact that remappings
// are not made in the numbers of vertices and modules
void read_clusters_without_remappings(
    std::istream& input,
    input_interface& inp_interf );

}  // gecmi

#endif  // CLUSTER_READER__CLUSTER_READER_HPP_
