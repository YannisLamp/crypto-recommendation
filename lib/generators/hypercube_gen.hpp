#ifndef LSH_NN_HYPERCUBE_GEN_H
#define LSH_NN_HYPERCUBE_GEN_H

#include <string>
#include <vector>

#include "hash_generator.hpp"
#include "cosine_h_gen.hpp"
#include "../data_structures/cust_vector.hpp"

/*
 * Hypercube Gen
 *
 * Class used to calculate the cosine g hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * General class for Hypercube algorithm
 * Accepts a vector of HashGenerators that need to return 0 and 1 values
 * Creates a hash from all those values
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */

template <typename dim_type>
class HypercubeGen : public HashGenerator<dim_type> {
private:
    std::vector< HashGenerator<dim_type>* > fFunctions;

public:
    HypercubeGen(std::vector< HashGenerator<dim_type>* > inFFunctions);
    // Delete contents of fFunctions vector (hash generator objects)
    ~HypercubeGen();

    int generate(CustVector<dim_type>* hashTarget);

    // No detailed hashes in this hash generator, but must implement "interface"
    bool hasDetailedHash();
    std::unordered_map<std::string, std::vector<int>>* getDetailedHashes();

    // Get size of object in bytes
    unsigned long getSize();
};


/*
* Template method definitions
*/

template <typename dim_type>
HypercubeGen<dim_type>::HypercubeGen(std::vector< HashGenerator<dim_type>* > inFFunctions) {
    fFunctions = inFFunctions;
}


template <typename dim_type>
HypercubeGen<dim_type>::~HypercubeGen() {
    for (int i = 0; i < fFunctions.size(); i++)
        delete fFunctions[i];
}


template <typename dim_type>
int HypercubeGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    int hash_num = 0;
    for (int i = 0; i < fFunctions.size(); i++) {
        hash_num = hash_num << 1;

        int hi = fFunctions[i]->generate(hashTarget);
        hash_num = hash_num + hi;
    }

    return hash_num;
}


template <typename dim_type>
bool HypercubeGen<dim_type>::hasDetailedHash() { return false; }


template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* HypercubeGen<dim_type>::getDetailedHashes() { return nullptr; }


template <typename dim_type>
unsigned long HypercubeGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + fFunctions.capacity()*sizeof(HashGenerator<dim_type>*);
    for (int i = 0; i < fFunctions.size(); i++) {
        size = size + fFunctions[i]->getSize();
    }

    return size;
}

#endif //LSH_NN_HYPERCUBE_GEN_H
