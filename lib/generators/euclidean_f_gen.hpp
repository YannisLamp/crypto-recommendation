#ifndef LIB_EUCLIDEAN_F_GEN_H
#define LIB_EUCLIDEAN_F_GEN_H

#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_map>

#include "hash_generator.hpp"
#include "euclidean_h_gen.hpp"
#include "../data_structures/cust_vector.hpp"

/*
 * Euclidean F
 *
 * Class used to calculate the euclidean f hash value of a given euclidean phi hash
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Creates an EuclideanHGen object, which calls during its hash generation process
 *
 * Stores all previous hashes of each vector that it generates a hash for in a map so as to be concise
 *
 * Templated, so that it can generate hashes for any type of vector (int, float type dimensions)
 */

template <typename dim_type>
class EuclideanFGen : public HashGenerator<dim_type> {
private:
    EuclideanHGen<dim_type>* hGenerator;
    std::default_random_engine* rand_generator;
    std::unordered_map<int, int> num_to_bin_hashes;

public:
    EuclideanFGen(int dim_num, float in_w, std::default_random_engine* rand_gen);
    ~EuclideanFGen();

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
EuclideanFGen<dim_type>::EuclideanFGen(int dim_num, float in_w, std::default_random_engine* rand_gen) {
    rand_generator = rand_gen;
    hGenerator = new EuclideanHGen<dim_type>(dim_num, in_w, rand_gen);
}

template <typename dim_type>
EuclideanFGen<dim_type>::~EuclideanFGen() {
    delete hGenerator;
}

template <typename dim_type>
int EuclideanFGen<dim_type>::generate(CustVector<dim_type>* hashTarget) {
    int hash_result;
    int hash_num = hGenerator->generate(hashTarget);
    if (num_to_bin_hashes.count(hash_num) == 1) {
        hash_result = num_to_bin_hashes[hash_num];
    }
    else {
        std::uniform_int_distribution<int> uni_distribution(1, 2);
        hash_result = mod(hash_num, uni_distribution(*rand_generator));

        num_to_bin_hashes.emplace(hash_num, hash_result);
    }

    return hash_result;
}

template <typename dim_type>
bool EuclideanFGen<dim_type>::hasDetailedHash() { return false; }


template <typename dim_type>
std::unordered_map<std::string, std::vector<int>>* EuclideanFGen<dim_type>::getDetailedHashes() { return nullptr; }


template <typename dim_type>
unsigned long EuclideanFGen<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + hGenerator->getSize();
    size = size + sizeof(rand_generator);
    size = size + num_to_bin_hashes.size() * 2*sizeof(int);

    return size;
}


#endif //LSH_NN_EUCLIDEAN_F_GEN_H
