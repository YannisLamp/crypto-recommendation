#ifndef CRYPTO_REC_HPP
#define CRYPTO_REC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "./data_structures/cust_vector.hpp"
#include "./data_structures/tweet.h"
#include "lsh_cube.hpp"


/*
 * Different essential functions used for cryptocurrency recommendation
 *
 * This file also contains the declarations and definitions for functions that needed to be templated
 *
 */



// Create and return different CustVector objects, each representing a user, from an input tweet map
template <typename dim_type>
std::vector< CustVector<dim_type> > tweets_to_user_vectors(std::unordered_map<std::string, Tweet>& tweets, int crypto_num);

template <typename dim_type>
std::vector< CustVector<dim_type> > clusters_to_user_vectors();

// Returns a vector parallel to the input neighbors vector that contains the cosine similarity of each user pair
// The neighbors vector is sorted and filtered in this function
template <typename dim_type>
std::vector<double> get_P_closest(std::vector< CustVector<dim_type>* >& neighbors, CustVector<dim_type>& user, int P);

// Parralel quicksort implementation
template <typename dim_type>
void parallel_quickSort(std::vector<double>& sim, std::vector< CustVector<dim_type>* >& neighbors, int low, int high);

// Parallel partition implementation, to be used in parallel quicksort for cosine similarities and neighbors
template <typename dim_type>
int parralel_partition(std::vector<double>& sim, std::vector< CustVector<dim_type>* >& neighbors, int low, int high);

/*
* Template utility function definitions
*/



template <typename dim_type>
std::vector< CustVector<dim_type> > tweets_to_user_vectors(std::unordered_map<std::string, Tweet>& tweets, int crypto_num) {
    std::unordered_map< std::string, std::vector<dim_type> > user_map;
    std::unordered_map< std::string, std::vector<int> > known_index_map;

    // For each tweet,
    for (auto& tweet : tweets) {
        std::string user_id = tweet.second.getUserId();
        std::vector<int> crypto_indexes = tweet.second.getCryptoIndexes();
        double score = tweet.second.getSentimentScore();

        if (user_map.count(user_id) == 0) {
            std::vector<dim_type> user_vector(crypto_num);
            user_map.emplace(user_id, user_vector);

            std::vector<int> is_unknown(crypto_num);
            known_index_map.emplace(user_id, is_unknown);
        }

        for (auto index : crypto_indexes) {
            if (score > 0)
                user_map[user_id][index] = user_map[user_id][index] + score;

            known_index_map[user_id][index] = 1;
        }
    }

    // Create CustVector objects to represent each user
    std::vector< CustVector<dim_type> > user_vectors;
    for (auto& user : user_map) {
        // Create set of unknown cryptocurrency indexes and calculate vector mean
        double sum = 0;
        int known_number = 0;
        std::set<int> unknown_indexes;
        bool useless = true;
        for (int i = 0; i < user.second.size(); i++) {
            if (known_index_map[user.first][i] == 0) {
                unknown_indexes.emplace(i);
            }
            else {
                sum = sum + user.second[i];
                known_number++;
            }

            if (user.second[i] != 0)
                useless = false;
        }

        // Filter vectors that we know nothing of
        if (!useless) {
            double mean = sum / known_number;

            // Replace unknown cryptocurrency scores with mean
            for (int index : unknown_indexes)
                user.second[index] = mean;

            CustVector<dim_type> current_user(user.first, user.second, unknown_indexes, mean);
            user_vectors.emplace_back(current_user);
        }
    }

    return user_vectors;
}


template <typename dim_type>
std::vector<double> get_P_closest(std::vector< CustVector<dim_type>* >& neighbors, CustVector<dim_type>& user, int P) {
    // Cache cosine similarities in vector
    std::vector<double> similarities(neighbors.size());

    // Calculate and store similarities
    for (int i = 0; i < neighbors.size(); i++)
        similarities[i] = neighbors[i]->cosineSimilarity(&user);

    // Parallel quicksort algorithm for the two vectors (neighbors and similarities
    parallel_quickSort(similarities, neighbors, 0, similarities.size()-1);

    if (neighbors.size() > P) {
        neighbors.resize(P);
        similarities.resize(P);
    }

    return similarities;
}


template <typename dim_type>
int parralel_partition(std::vector<double>& sim, std::vector< CustVector<dim_type>* >& neighbors, int low, int high) {
    double pivot = sim[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++) {
        // If current element is greater than or equal to pivot
        if (sim[j] >= pivot) {
            i++;

            // Parallel swap
            double temp_sim = sim[i];
            sim[i] = sim[j];
            sim[j] = temp_sim;

            CustVector<dim_type>* temp_neigh = neighbors[i];
            neighbors[i] = neighbors[j];
            neighbors[j] = temp_neigh;
        }
    }

    double temp_sim = sim[i + 1];
    sim[i + 1] = sim[high];
    sim[high] = temp_sim;

    CustVector<dim_type>* temp_neigh = neighbors[i + 1];
    neighbors[i + 1] = neighbors[high];
    neighbors[high] = temp_neigh;

    return (i + 1);
}


// Parralel quicksort implementation
template <typename dim_type>
void parallel_quickSort(std::vector<double>& sim, std::vector< CustVector<dim_type>* >& neighbors, int low, int high) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = parralel_partition(sim, neighbors, low, high);

        // Separately sort elements before
        // partition and after partition
        parallel_quickSort(sim, neighbors, low, pi - 1);
        parallel_quickSort(sim, neighbors, pi + 1, high);
    }
}


#endif //CRYPTO_REC_HPP
