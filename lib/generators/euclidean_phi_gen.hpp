#ifndef LIB_EUCLIDEAN_PHI_GEN_H
#define LIB_EUCLIDEAN_PHI_GEN_H

#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_map>

#include "hash_generator.hpp"
#include "euclidean_h_gen.hpp"
#include "../data_structures/cust_vector.hpp"


/*
 * Euclidean Phi
 *
 * Class used to calculate the euclidean phi hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Creates an input number of EuclideanHGen objects, which calls during its hash
 * generation process
 *
 * Stores all the detailed hashes of each vector that it generates a hash for,
 * to use later for easier initial comparison between vectors
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */

template <typename dim_type>
class EuclideanPhiGen : public HashGenerator<dim_type> {
private:
    std::vector< EuclideanHGen<dim_type>* > hFunctions;
    std::vector<int> rs;
    int M;

    std::unordered_map<std::string, std::vector<int>> id_to_det_hashes;

public:
    EuclideanPhiGen(int k, int dim_num, float in_w, std::default_random_engine* rand_generator);
    ~EuclideanPhiGen();

    int generate(CustVector<dim_type>* hashTarget);

    // Uses EuclideanHGen generators to create a hash
    // The hashes these generators provide are the detailed hash
    bool hasDetailedHash();
    std::unordered_map<std::string, std::vector<int>>* getDetailedHashes();

    // Get size of object in bytes
    unsigned long getSize();
};


/*
* Template method definitions
*/

template <typename dim_type>
EuclideanPhiGen<dim_type>::EuclideanPhiGen(int k, int dim_num, float in_w, std::default_random_engine* rand_generator) {
    hFunctions.reserve(k);
    rs.reserve(k);

    std::uniform_int_distribution<int> uni_int_dist(0, 100);
    for (int i = 0; i < k; i++) {
        hFunctions.emplace_back( new EuclideanHGen<dim_type>(dim_num, in_w, rand_generator) );
        rs.emplace_back( uni_int_dist(*rand_generator) );
    }

    M = int( pow(2, 32) - 5 );
}


template <typename dim_type>
EuclideanPhiGen<dim_type>::~EuclideanPhiGen() {
    for (int i = 0; i < hFunctions.size(); i++)
        delete hFunctions[i];
}


template <typename dim_type>
int EuclideanPhiGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    std::vector<int>detailed_hash;

    unsigned int hash_num = 0;
    for (int i = 0; i < hFunctions.size(); i++) {
        int hi = hFunctions[i]->generate(hashTarget);
        long temp = hi * rs[i];
        hash_num = hash_num + mod(temp, M);

        detailed_hash.emplace_back(hi);
    }

    id_to_det_hashes.emplace(hashTarget->getId(), detailed_hash);

    return mod(hash_num, M);
}


template <typename dim_type>
bool EuclideanPhiGen<dim_type>::hasDetailedHash() { return true; }


template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* EuclideanPhiGen<dim_type>::getDetailedHashes() { return &id_to_det_hashes; }


template <typename dim_type>
unsigned long EuclideanPhiGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + hFunctions.capacity()*sizeof(EuclideanHGen<dim_type>*);
    size = size + rs.capacity()*sizeof(int);
    for (int i = 0; i < hFunctions.size(); i++) {
        size = size + hFunctions[i]->getSize();
    }

    return size;
}

#endif //LIB_EUCLIDEAN_PHI_GEN_H
