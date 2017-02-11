#ifndef CLUSTER_READER__CLUSTER_READER_HPP_
#define CLUSTER_READER__CLUSTER_READER_HPP_

#include <istream>


namespace gecmi {

// Input interface...
class input_interface {
public:
    virtual void add_vertex_module( size_t internal_vertex_id, size_t module_id ) = 0;
    virtual void reserve_vertices_modules( size_t vertices_num=0, size_t modules_num=0 ) = 0;
    virtual void shrink_to_fit_modules() = 0;
    virtual size_t uniqlSize() const=0;
    virtual size_t uniqrSize() const=0;
public:
    virtual ~input_interface() = default;
};

// The name of this method reflects the fact that remappings
// are not made in the numbers of vertices and modules
size_t read_clusters_without_remappings(std::istream& input,
    input_interface& inp_interf, const char* fname=nullptr,
    float membership=1.f);  // Average expected membership
}  // gecmi

#endif  // CLUSTER_READER__CLUSTER_READER_HPP_
