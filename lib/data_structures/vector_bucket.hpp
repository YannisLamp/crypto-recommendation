#ifndef LIB_VECTOR_BUCKET_H
#define LIB_VECTOR_BUCKET_H

#include <vector>

#include "cust_vector.hpp"

/*
 * Vector Bucket
 *
 * Vector bucket class used by CustHashtable, in order to store input vectors
 *
 * Just a container, at this point it does not handle any operations, as everything is handled by
 * the CustHashtable class
 *
 * Templated, so that it can store any type of vector (int, float type dimensions)
 */


template <typename dim_type>
class VectorBucket {
private:
    std::vector< CustVector<dim_type>* > vectors;

public:
    void insertVector(CustVector<dim_type>* inVector);
    void insertVector(CustVector<dim_type>* inVector, std::vector<int> detHashes);

    std::vector< CustVector<dim_type>* >* getVectors();

    // Get size of object in bytes
    unsigned long getSize();
};


/*
* Template method definitions
*/


template <typename dim_type>
void VectorBucket<dim_type>::insertVector(CustVector<dim_type>* inVector) {
    vectors.emplace_back(inVector);
}


template <typename dim_type>
void VectorBucket<dim_type>::insertVector(CustVector<dim_type>* inVector, std::vector<int> in_hashes) {
    vectors.emplace_back(inVector);
}


template <typename dim_type>
std::vector< CustVector<dim_type>* >* VectorBucket<dim_type>::getVectors() { return &vectors; }


template <typename dim_type>
unsigned long VectorBucket<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + vectors.capacity()*sizeof(CustVector<dim_type>*);

    return size;
}

#endif //LIB_VECTOR_BUCKET_H
