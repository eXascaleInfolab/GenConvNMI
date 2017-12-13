#ifndef CLUSTER_READER__CLUSTER_READER_HPP_
#define CLUSTER_READER__CLUSTER_READER_HPP_

#include <istream>
#include <unordered_map>


namespace gecmi {

using std::unordered_map;


// Mapping of ids to provide solid range starting from 0 if required
typedef unsigned Id;  // Use unsigned to reduce the memory consumption on remapping
typedef unordered_map<Id, Id>  IdMap;

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

size_t read_clusters(std::istream& input,
    input_interface& inp_interf, const char* fname=nullptr,
    IdMap* idmap=nullptr, float membership=1.f,  // Average expected membership
    bool fltdups=true, size_t* nmods=nullptr);  // Filter out duplicates of clusters
}  // gecmi

#endif  // CLUSTER_READER__CLUSTER_READER_HPP_
