#ifndef LIB_HASH_GENERATOR_H
#define LIB_HASH_GENERATOR_H

#include <unordered_map>

#include "../data_structures/cust_vector.hpp"


/*
 * Hash Generator
 *
 * Abstract Class used to calculate the euclidean phi hash value of a given vector
 * Implements the HashGenerator "interface", accepts CustVector objects
 *
 * Templated, so that it can "generate" hashes for any type of vector (int, float type dimensions)
 */


template <typename dim_type>
class HashGenerator {

public:
    virtual ~HashGenerator() = 0;

    virtual int generate(CustVector<dim_type>*) = 0;
    virtual bool hasDetailedHash() = 0;
    virtual std::unordered_map<std::string, std::vector<int>>* getDetailedHashes() = 0;

    // Get size of object in bytes
    virtual unsigned long getSize() = 0;
};

template <typename dim_type>
HashGenerator<dim_type>::~HashGenerator() {
    // Empty but NEEDS to be there
}


#endif //LIB_HASH_GENERATOR_H
