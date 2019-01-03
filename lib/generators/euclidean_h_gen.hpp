#ifndef LIB_EUCLIDEAN_H_GEN_H
#define LIB_EUCLIDEAN_H_GEN_H

#include <string>
#include <vector>
#include <random>
#include <numeric>
#include <cmath>

#include "hash_generator.hpp"
#include "../data_structures/cust_vector.hpp"

#include "../utils.hpp"


/*
 * Euclidean H
 *
 * Class used to calculate the euclidean h hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Creates an input number of random CustVectors with normally distributed dimensions
 * and a random t integer
 *
 * As it does not use any other hash, does not do any sort of hash aggregation,
 * it does not have the need to store any detailed hashes
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */

template <typename dim_type>
class EuclideanHGen : public HashGenerator<dim_type> {
private:
    CustVector<float> *v;
    float t;
    float w;

public:
    EuclideanHGen(int dim_num, float in_w, std::default_random_engine* rand_generator);
    ~EuclideanHGen();

    int generate(CustVector<dim_type> *hashTarget);

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
EuclideanHGen<dim_type>::EuclideanHGen(int dim_num, float in_w, std::default_random_engine* rand_generator): w(in_w) {
    // Create v (normal distributed float dimensions)
    std::normal_distribution<float> norm_distribution(0, 1);
    std::vector<float> temp;
    temp.reserve(dim_num);
    for (int i = 0; i < dim_num; i++)
        temp.emplace_back( norm_distribution(*rand_generator) );
    v = new CustVector<float>("v", temp);

    // Create t (uniformly distributed float)
    std::uniform_real_distribution<float> uni_distribution(0, w);
    t = uni_distribution(*rand_generator);
}


template <typename dim_type>
EuclideanHGen<dim_type>::~EuclideanHGen() {
    delete v;
}

template <typename dim_type>
int EuclideanHGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    long double inner_prod = v->inner_product<dim_type>(hashTarget, 0.0);
    return int( floor( (inner_prod + t) / w ) );
}

// This hash generator does not have any sort of detailed hash and does not do any aggregation from other hashes
template <typename dim_type>
bool EuclideanHGen<dim_type>::hasDetailedHash() { return false; }

// This method exists just to implement the interface
template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* EuclideanHGen<dim_type>::getDetailedHashes() { return nullptr; }


template <typename dim_type>
unsigned long EuclideanHGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + sizeof(*v);

    return size;
}

#endif //LIB_EUCLIDEAN_H_GEN_H
