#ifndef LSH_CUBE_HPP
#define LSH_CUBE_HPP

#include <iostream>
#include <string>
#include <set>

#include <chrono>
#include <random>

#include "./data_structures/cust_vector.hpp"
#include "./data_structures/cust_hashtable.hpp"
#include "./generators/euclidean_phi_gen.hpp"
#include "./generators/cosine_g_gen.hpp"
#include "./generators/euclidean_f_gen.hpp"
#include "./generators/hypercube_gen.hpp"

#include "utils.hpp"

/*
 * Previous project's lsh and hypercube code, (initialization, utilities) repurposed into different functions and
 * aggregated here so that they can be freely used for clustering in this project
 *
 * All functions are templated, so they can be used for any kind of input vector type
 */


template <typename vector_type>
std::vector< CustHashtable<vector_type>* > create_LSH_hashtables(std::vector< CustVector<vector_type> >& input_vectors,
        std::string metric_type, int k, int L, int lsh_bucket_div, double euclidean_h_w);

template <typename vector_type>
std::vector< CustVector<vector_type>* > get_LSH_combined_buckets(std::vector< CustHashtable<vector_type>* >& lshHashtables,
        CustVector<vector_type>* queryVec);

/*
* Function definitions
*/

template <typename vector_type>
std::vector< CustHashtable<vector_type>* > create_LSH_hashtables(std::vector< CustVector<vector_type> >& input_vectors,
        const std::string metric_type, int k, int L, int lsh_bucket_div, double euclidean_h_w) {

    // Create L Hashtables and insert those vectors in them using H
    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rand_generator;
    rand_generator.seed(seed);

    std::vector< CustHashtable<vector_type>* > lshHashtables;
    lshHashtables.reserve(L);
    for (int i = 0; i < L; i++) {
        // If the chosen metric is euclidean, then create an EuclideanPhiGen hash generator
        // and pass in to the hashtable constructor
        if (metric_type == "euclidean") {
            EuclideanPhiGen<vector_type>* generator = new EuclideanPhiGen<vector_type>(k, input_vectors[0].getDimNumber(), euclidean_h_w, &rand_generator);
            lshHashtables.emplace_back(new CustHashtable<vector_type>(generator, input_vectors.size() / lsh_bucket_div));
        }
        // Else create a CosineGGen hash generator
        else if (metric_type == "cosine") {
            CosineGGen<vector_type>* generator = new CosineGGen<vector_type>(k, input_vectors[0].getDimNumber(), &rand_generator);
            int bucket_num = int( pow(2, k) );
            lshHashtables.emplace_back( new CustHashtable<vector_type>(generator, bucket_num));
        }
        // Insert all read vectors into the new LSH hashtable
        for (int vec_i = 0; vec_i < input_vectors.size(); vec_i++)
            lshHashtables[i]->insertVector(&input_vectors[vec_i]);
    }

    return lshHashtables;
}


template <typename vector_type>
std::vector< CustVector<vector_type>* > get_LSH_combined_buckets(std::vector< CustHashtable<vector_type>* >& lshHashtables,
        CustVector<vector_type>* queryVec) {
    std::set< CustVector<vector_type>* > buckets;
    // Put all different vectors of chosen bucket for each lsh hashtable into a set
    for (int i = 0; i < lshHashtables.size(); i++) {
        std::vector< CustVector<vector_type>* > filtered_bucket = lshHashtables[i]->getBucketFor(queryVec);
        std::copy( filtered_bucket.begin(), filtered_bucket.end(), std::inserter( buckets, buckets.end() ) );
    }

    // Convert set to vector and return
    std::vector< CustVector<vector_type>* > output(buckets.begin(), buckets.end());
    return output;
}


template <typename vector_type>
CustHashtable<vector_type>* create_hypercube(std::vector< CustVector<vector_type> >& input_vectors,
        const std::string metric_type, int k, double euclidean_h_w) {

    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rand_generator;
    rand_generator.seed(seed);

    std::vector< HashGenerator<vector_type>* > generators;
    // If the chosen metric is euclidean, then create EuclideanFGen hash generators to pass to HypercubeGen constructor
    if (metric_type == "euclidean") {
        for (int i = 0; i < k; i++)
            generators.emplace_back(new EuclideanFGen<vector_type>(input_vectors[0].getDimNumber(), euclidean_h_w, &rand_generator));
    }
        // Else create a CosineGGen hash generator
    else if (metric_type == "cosine") {
        for (int i = 0; i < k; i++)
            generators.emplace_back(new CosineHGen<vector_type>(input_vectors[0].getDimNumber(), &rand_generator));
    }
    // Create the Hypercube
    int bucket_num = int( pow(2, k) );
    CustHashtable<vector_type>* hypercube = new CustHashtable<vector_type>(new HypercubeGen<vector_type>(generators), bucket_num);

    // Insert all read vectors into the Hypercube
    for (int vec_i = 0; vec_i < input_vectors.size(); vec_i++)
        hypercube->insertVector(&input_vectors[vec_i]);

    return hypercube;
}


template <typename vector_type>
std::vector< CustVector<vector_type>* > get_hypercube_combined_buckets(CustHashtable<vector_type>& hypercube,
        CustVector<vector_type>* queryVec, int probes, int k) {

    // Convert set to vector and return
    int correct_index = hypercube.getHash(queryVec);
    std::vector< CustVector<vector_type>* > buckets = hypercube.getBucketFromIndex(correct_index);
    std::vector<int> hamming_neigh;
    int hamming_neigh_i = 0;
    if (probes > 1) {
        hamming_neigh = get_num_hamming_dist_from(correct_index, 1, 0, k);
    }

    int curr_probes = probes;
    //int curr_M = M;
    int curr_hamming_dist = 1;
    while (curr_probes > 0) {
        // If there are buckets with current hamming distance left
        if (hamming_neigh_i < hamming_neigh.size()) {
            std::vector< CustVector<vector_type>* > temp_bucket = hypercube.getBucketFromIndex( hamming_neigh[hamming_neigh_i] );
            // Merge buckets
            buckets.insert(buckets.end(), temp_bucket.begin(), temp_bucket.end());

            hamming_neigh_i++;
            curr_probes--;
        }
        // Else get new neighbors with greater hamming distance
        else {
            curr_hamming_dist++;
            hamming_neigh = get_num_hamming_dist_from(correct_index, curr_hamming_dist, 0, k);
            hamming_neigh_i = 0;
            // In this case we have searched the whole cube
            if (hamming_neigh.empty())
                break;
        }
    }

    return buckets;
}


#endif //LSH_CUBE_HPP