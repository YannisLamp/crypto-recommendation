#ifndef CLUSTER_INITIALIZATION_H
#define CLUSTER_INITIALIZATION_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <chrono>
#include <random>

#include "../data_structures/cust_vector.hpp"


/*
 * Functions used to implement various initialization algorithms required for vector clustering
 * Namely, random selection and k-means++
 *
 * All functions are templated, so they can be used for any kind of input vector type
 */


// Function that selects random k unique vectors uniformly from input to serve as initial centroids
template <typename vector_type>
std::vector< CustVector<vector_type>* > rand_selection(std::vector< CustVector<vector_type> >& input_vectors, int cluster_num);

// Function uses the k-means++ algorithm to select initial centroids
// Basically, in each iteration, pick a new centroid randomly among input vectors, but with probability proportional to
// the distance of that vector to its closest centroid (that have been previously selected)
template <typename vector_type>
std::vector< CustVector<vector_type>* > k_means_pp(std::vector< CustVector<vector_type> >& input_vectors, int cluster_num,
                                                   std::string metric_type);


/*
* Function definitions
*/

template <typename vector_type>
std::vector< CustVector<vector_type>* > rand_selection(std::vector< CustVector<vector_type> >& input_vectors, int cluster_num) {
    // Uniform integer random stuff and rand_generator initialization
    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rand_generator;
    rand_generator.seed(seed);
    std::uniform_int_distribution<int> uni_int_dist(0, input_vectors.size()-1);
    int rand_i = uni_int_dist(rand_generator);

    // Random centroids to return
    std::vector< CustVector<vector_type>* > centroids(cluster_num);
    centroids[0] = &input_vectors[rand_i];
    for (int i = 1; i < cluster_num; i++) {
        rand_i = uni_int_dist(rand_generator);
        int centroid_i = 0;
        while (centroid_i < i) {
            // If the same centroid happens to be chosen randomly a second time, then choose another and search again
            if (input_vectors[rand_i].getId() == centroids[centroid_i]->getId()) {
                rand_i = uni_int_dist(rand_generator);
                centroid_i = 0;
            }
            else
                centroid_i++;
        }

        centroids[i] = &input_vectors[rand_i];
    }

    return centroids;
}


template <typename vector_type>
std::vector< CustVector<vector_type>* > k_means_pp(std::vector< CustVector<vector_type> >& input_vectors, int cluster_num,
        std::string metric_type) {
    // Uniform integer random stuff and rand_generator initialization
    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rand_generator;
    rand_generator.seed(seed);
    std::uniform_int_distribution<int> uni_int_dist(0, input_vectors.size()-1);
    int rand_i = uni_int_dist(rand_generator);

    // Centroids to return
    // Initially, pick a random vector as the first centroid to start the main algorithm
    std::vector<CustVector<vector_type>* > centroids(cluster_num);
    centroids[0] = &input_vectors[rand_i];

    // Cache distances that have already been calculated
    std::unordered_map<std::string, double> distanceMap;

    std::vector<double> min_dists(input_vectors.size());
    for (int i = 1; i < cluster_num; i++) {
        double max_for_normalizing = 0;

        for (int vector_i = 0; vector_i < input_vectors.size(); vector_i++) {
            // Find min distance from cetroids
            double min = -1;
            for (int centroid_i = 0; centroid_i < i; centroid_i++) {
                std::string key = input_vectors[vector_i].getId() + "to" + centroids[centroid_i]->getId();

                double distance = 0;
                if (distanceMap.count(key) == 1) {
                    distance = distanceMap[key];
                }
                else {
                    if (metric_type == "euclidean")
                        distance = input_vectors[vector_i].euclideanDistance(centroids[centroid_i]);
                    else if (metric_type == "cosine")
                        distance = input_vectors[vector_i].cosineDistance(centroids[centroid_i]);
                    distanceMap[key] = distance;
                }

                if (min == -1 || distance < min)
                    min = distance;
            }

            min_dists[vector_i] = min;

            if (min > max_for_normalizing)
                max_for_normalizing = min;
        }

        // Divide all minimums with the one with the greates value for normalizing, square them and
        // for each one, add the previous to its value
        min_dists[0] = min_dists[0] / max_for_normalizing;
        min_dists[0] = min_dists[0] * min_dists[0];
        for (int min_i = 1; min_i < min_dists.size(); min_i++) {
            min_dists[min_i] = min_dists[min_i] / max_for_normalizing;
            min_dists[min_i] = min_dists[min_i] * min_dists[min_i];
            min_dists[min_i] = min_dists[min_i] + min_dists[min_i-1];
        }

        // Choose random double and perform a binary search to find the input vector whose area corresponds to it
        std::uniform_real_distribution<double> uni_double_dist(0.0, min_dists[min_dists.size()-1]);
        double rand_dis = uni_double_dist(rand_generator);
        int left = 0;
        int right = min_dists.size()-1;
        // Custom binary search, basically search for the two numbers that the random float can be put between
        // When found, exit and return the index of the right one, only if the random number is not lower than the first
        int chosen_i = 0;
        if (rand_dis > min_dists[left]) {
            while (right - left > 1) {
                int m = left + (right - left) / 2;

                if (rand_dis <= min_dists[m])
                    right = m;
                else
                    left = m;
            }
            chosen_i = right;
        }

        // Add chosen centroid
        centroids[i] = &input_vectors[chosen_i];
    }

    return centroids;
}


#endif //CLUSTER_INITIALIZATION_H
