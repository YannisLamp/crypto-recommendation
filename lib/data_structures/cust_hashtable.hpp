#ifndef LIB_CUST_HASHTABLE_H
#define LIB_CUST_HASHTABLE_H

#include <vector>

#include "../generators/hash_generator.hpp"
#include "../data_structures/vector_bucket.hpp"

/*
 * Custom Hashtable
 *
 * Hashtable class used for LSH to store vectors in buckets
 * Therefore, contains a given number of buckets and a hash generator to determine
 * in which bucket an input vector should be placed
 *
 * Accepts any HashGenerator object as its hash generator, so that it can be used for
 * any metric we want to use
 *
 * Templated, so that it can store any type of vector (int, float type dimensions)
 */


template <typename dim_type>
class CustHashtable {
private:
    HashGenerator<dim_type>* hashGenerator;
    std::vector< VectorBucket<dim_type>* > buckets;

public:
    CustHashtable(HashGenerator<dim_type>* inHashGen, int bucket_num);
    // Destructor also deletes input hash generator, other than the created buckets
    ~CustHashtable();

    int insertVector(CustVector<dim_type>* inVector);
    std::vector< CustVector<dim_type>* > getFilteredBucketFor(CustVector<dim_type>* queryVector);
    std::vector< CustVector<dim_type>* > getBucketFor(CustVector<dim_type>* queryVector);
    std::vector< CustVector<dim_type>* > getBucketFromIndex(int index);
    int getHash(CustVector<dim_type>* queryVector);

    // Get size of object in bytes
    unsigned long getSize();
};


/*
 * Template method definitions
 */


template <typename dim_type>
CustHashtable<dim_type>::CustHashtable(HashGenerator<dim_type>* inHashGen, int bucket_num) : hashGenerator(inHashGen) {
    for (unsigned int i = 0; i < bucket_num; i++)
        buckets.emplace_back( new VectorBucket<dim_type>() );
}


template <typename dim_type>
CustHashtable<dim_type>::~CustHashtable() {
    for (int i = 0; i < buckets.size(); i++)
        delete buckets[i];
    delete hashGenerator;
}


template <typename dim_type>
int CustHashtable<dim_type>::insertVector(CustVector<dim_type>* inVector) {
    // Mod should not matter if the hash as accurate
    unsigned int index = mod(hashGenerator->generate(inVector), buckets.size());
    buckets[index]->insertVector(inVector);
}


template <typename dim_type>
std::vector< CustVector<dim_type>* > CustHashtable<dim_type>::getFilteredBucketFor(CustVector<dim_type>* queryVector) {
    // Mod should not matter if the hash as accurate
    unsigned int index = mod(hashGenerator->generate(queryVector), buckets.size());

    std::vector< CustVector<dim_type>* > tempBucket = *( buckets[index]->getVectors() );
    std::vector< CustVector<dim_type>* > retBucket;

    if (hashGenerator->hasDetailedHash()) {
        std::unordered_map<std::string, std::vector<int>>* id_to_hashes = hashGenerator->getDetailedHashes();
        std::vector<int>* query_det_hash = &( (*id_to_hashes)[ queryVector->getId() ] );

        // Compare the detailed hash of each vector in the bucket with the query vector
        for (const auto& buckVector : tempBucket) {
            bool diff = false;
            for (unsigned int hash_i = 0; hash_i < query_det_hash->size(); hash_i++) {
                std::vector<int> curr_det_hash = (*id_to_hashes)[ buckVector->getId() ];
                if ( (curr_det_hash)[hash_i] != (*query_det_hash)[hash_i]) {
                    diff = true;
                    break;
                }
            }
            if (!diff)
                retBucket.emplace_back(buckVector);
        }
    }
    else
        retBucket = tempBucket;

    return retBucket;
}


template <typename dim_type>
std::vector< CustVector<dim_type>* > CustHashtable<dim_type>::getBucketFor(CustVector<dim_type>* queryVector) {
    // Mod should not matter if the hash is accurate
    unsigned int index = mod(hashGenerator->generate(queryVector), buckets.size());
    std::vector< CustVector<dim_type>* > retBucket =*( buckets[index]->getVectors() );

    return retBucket;
}


template <typename dim_type>
std::vector< CustVector<dim_type>* > CustHashtable<dim_type>::getBucketFromIndex(int index) {
    return *( buckets[index]->getVectors() );
}


template <typename dim_type>
int CustHashtable<dim_type>::getHash(CustVector<dim_type>* queryVector) {
    return mod(hashGenerator->generate(queryVector), buckets.size());
}


template <typename dim_type>
unsigned long CustHashtable<dim_type>::getSize() {
    unsigned long size = sizeof(*this);
    size = size + hashGenerator->getSize();
    size = size + buckets.capacity()*sizeof(VectorBucket<dim_type>*);
    for (int i = 0; i < buckets.size(); i++) {
        size = size + buckets[i]->getSize();
    }

    return size;
}

#endif //LIB_CUST_HASHTABLE_H
