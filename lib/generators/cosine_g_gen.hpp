#ifndef LSH_NN_COSINE_G_GEN_H
#define LSH_NN_COSINE_G_GEN_H

#include <string>
#include <vector>

#include "hash_generator.hpp"
#include "cosine_h_gen.hpp"
#include "../data_structures/cust_vector.hpp"

/*
 * Cosine G
 *
 * Class used to calculate the cosine g hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Creates an input number of CosineHGen objects, which calls during its hash
 * generation process
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */

template <typename dim_type>
class CosineGGen : public HashGenerator<dim_type> {
private:
    std::vector< CosineHGen<dim_type>* > hFunctions;

public:
    CosineGGen(int k, int dim_num, std::default_random_engine* rand_generator);
    ~CosineGGen();

    int generate(CustVector<dim_type>* hashTarget);

    // Creates a hash from given hash values, but the hash completely represents the hash values
    // So there is not need to store the detailed hashes
    bool hasDetailedHash();
    std::unordered_map<std::string, std::vector<int>>* getDetailedHashes();

    // Get size of object in bytes
    unsigned long getSize();
};


/*
* Template method definitions
*/

template <typename dim_type>
CosineGGen<dim_type>::CosineGGen(int k, int dim_num, std::default_random_engine* rand_generator) {
    hFunctions.reserve(k);
    for (int i = 0; i < k; i++)
        hFunctions.emplace_back( new CosineHGen<dim_type>(dim_num, rand_generator) );
}

template <typename dim_type>
CosineGGen<dim_type>::~CosineGGen() {
    for (int i = 0; i < hFunctions.size(); i++)
        delete hFunctions[i];
}

template <typename dim_type>
int CosineGGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    int hash_num = 0;
    for (int i = 0; i < hFunctions.size(); i++) {
        hash_num = hash_num << 1;

        int hi = hFunctions[i]->generate(hashTarget);
        hash_num = hash_num + hi;
    }

    return hash_num;
}

template <typename dim_type>
bool CosineGGen<dim_type>::hasDetailedHash() { return false; }

template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* CosineGGen<dim_type>::getDetailedHashes() { return nullptr; }


template <typename dim_type>
unsigned long CosineGGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + hFunctions.capacity()*sizeof(CosineHGen<dim_type>*);
    for (int i = 0; i < hFunctions.size(); i++) {
        size = size + hFunctions[i]->getSize();
    }

    return size;
}


#endif //LSH_NN_COSINE_G_GEN_H
