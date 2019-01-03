#ifndef LSH_NN_COSINE_H_GEN_H
#define LSH_NN_COSINE_H_GEN_H

#include <vector>
#include <random>

#include "hash_generator.hpp"
#include "../data_structures/cust_vector.hpp"

#include "../utils.hpp"

/*
 * Cosine H
 *
 * Class used to calculate the euclidean h hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Creates an input number of random CustVectors with normally distributed dimensions
 *
 * As it does not use any other hash, does not do any sort of hash aggregation,
 * it does not have the need to store any detailed hashes
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */


template <typename dim_type>
class CosineHGen : public HashGenerator<dim_type> {
private:
    CustVector<double>* r;

public:
    CosineHGen(int dim_num, std::default_random_engine* rand_generator);
    ~CosineHGen();

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
CosineHGen<dim_type>::CosineHGen(int dim_num, std::default_random_engine* rand_generator) {
    std::normal_distribution<double> distribution(0, 1);

    std::vector<double> temp;
    temp.reserve(dim_num);
    for (int i = 0; i < dim_num; i++)
        temp.emplace_back( distribution(*rand_generator) );
    r = new CustVector<double>("r", temp);
}

template <typename dim_type>
CosineHGen<dim_type>::~CosineHGen() {
    delete r;
}

template <typename dim_type>
int CosineHGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    long double inner_prod = r->inner_product<dim_type>(hashTarget, 0.0);

    if (inner_prod >= 0)
        return 1;
    else
        return 0;
}


// This hash generator does not have any sort of detailed hash and does not do any aggregation from other hashes
template <typename dim_type>
bool CosineHGen<dim_type>::hasDetailedHash() { return false; }


// This method exists just to implement the interface
template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* CosineHGen<dim_type>::getDetailedHashes() { return nullptr; }


template <typename dim_type>
unsigned long CosineHGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + sizeof(*r);

    return size;
}

#endif //LSH_NN_COSINE_H_GEN_H
