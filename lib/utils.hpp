#ifndef LIB_UTILS_H
#define LIB_UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

#include "./data_structures/cust_vector.hpp"


/*
 * Utility functions used in various algorithms
 *
 * This file also contains the declarations for functions that needed to be templated
 *
 */


// Split input string, with input delimiter and return a vector of the resulting strings
std::vector<std::string> split(const std::string& s, char delimiter);

// Recursive function that returns all the numbers that are a given hamming distance away from an input number
// When called if all possible combinations are wanted, min_bit should have the value zero
// Bits is the number of bits the input number is made of (for the cases of the binary hash functions)
std::vector<int> get_num_hamming_dist_from(int num, int dist, int min_bit, int bits);

// Split input string, with input delimiter and return a vector of the resulting strings converted
// the numbers are passed to the input conversion_f lambda
template <typename conv_type>
std::vector<conv_type> split_convert(const std::string& s, char delimiter,
        std::function<conv_type (const std::string&)> conversion_f);

// Custom mod implementation, x mod n
template <typename x_type, typename n_type>
int mod(x_type x, n_type n);

// Calculate the minimum euclidean distance of a given vector of vectors and a query vector using exhaustive search
template <typename dim_type>
double min_vector_euclidean_dist(CustVector<dim_type>* queryVector, std::vector< CustVector<dim_type> >* vectors);

// Calculate the minimum cosine distance of a given vector of vectors and a query vector using exhaustive search
template <typename dim_type>
dim_type min_vector_cosine_dist(CustVector<dim_type>* queryVector, std::vector< CustVector<dim_type> >* vectors);

// From a given input clusters, return a vector of clusters, with each cluster containing a pointer to every CustomVector
// that belongs to it
template <typename dim_type>
std::vector< std::vector< CustVector<dim_type>* > > separate_clusters_from_input(std::vector< CustVector<dim_type> >& in_vectors,
        int cluster_num);

// For every vector in input vectors, remove it from the cluster it belongs to
template <typename dim_type>
void remove_clustering(std::vector< CustVector<dim_type> >& in_vectors);

// Find minimum distance of different input vectors
template <typename vector_type>
double find_min_vector_distance(std::vector< CustVector<vector_type>* >& vectors, std::string metric_type);

// Convert file contents to argument format (to be given to argParser)
// Mainly used for csv parsing
std::vector<std::string> file_to_args(std::string filename, char delimiter);


/*
* Template utility function definitions
*/


template <typename conv_type>
std::vector<conv_type> split_convert(const std::string& s, char delimiter,
        std::function<conv_type (const std::string&)> conversion_f) {
    std::vector<conv_type> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
        tokens.emplace_back( conversion_f(token) );
    return tokens;
}


template <typename x_type, typename n_type>
int mod(x_type x, n_type n) { return (x % n + n) % n; }


template <typename a_type, typename b_type>
bool f_equals(a_type a, b_type b, double epsilon) {
    return std::abs(a - b) < epsilon;
}


template <typename q_type, typename dim_type>
double min_vector_euclidean_dist(CustVector<q_type>* queryVector, std::vector< CustVector<dim_type> >* vectors) {
    if (vectors->size() == 0)
        return 0;
    CustVector<dim_type>* currVector = & (*vectors)[0];

    double min_dist = queryVector->template euclideanDistance<dim_type>(currVector);
    for (int i = 1; i < vectors->size(); i++) {
        currVector = & (*vectors)[i];
        double curr_dist = queryVector->template euclideanDistance<dim_type>(currVector);
        if (curr_dist < min_dist)
            min_dist = curr_dist;
    }

    return min_dist;
}


template <typename q_type, typename dim_type>
double min_vector_cosine_dist(CustVector<q_type>* queryVector, std::vector< CustVector<dim_type> >* vectors) {
    if (vectors->size() == 0)
        return 0;
    CustVector<dim_type>* currVector = & (*vectors)[0];

    double min_dist = queryVector->template cosineDistance<dim_type>(currVector);
    for (int i = 1; i < vectors->size(); i++) {
        currVector = & (*vectors)[i];
        double curr_dist = queryVector->template cosineDistance<dim_type>(currVector);
        if (curr_dist < min_dist)
            min_dist = curr_dist;
    }

    return min_dist;
}


template <typename dim_type>
void remove_clustering(std::vector< CustVector<dim_type> >& in_vectors) {
    for (int i = 0; i < in_vectors.size(); i++)
        in_vectors[i].resetCluster();
}


template <typename dim_type>
std::vector< std::vector< CustVector<dim_type>* > > separate_clusters_from_input(std::vector< CustVector<dim_type> >& in_vectors,
        int cluster_num) {
    std::vector< std::vector< CustVector<dim_type>* > > clusters(cluster_num);
    for (int i = 0; i < in_vectors.size(); i++)
        clusters[in_vectors[i].getCluster()].emplace_back(&in_vectors[i]);

    return clusters;
}


template <typename vector_type>
double find_min_vector_distance(std::vector< CustVector<vector_type>* >& vectors, std::string metric_type) {
    double min_distance = -1;
    for (int centroid_i = 0; centroid_i < vectors.size(); centroid_i++) {
        for (int i = centroid_i + 1; i < vectors.size(); i++) {
            double centroid_distance = 0;
            if (metric_type == "euclidean")
                centroid_distance = vectors[centroid_i]->euclideanDistance(vectors[i]);
            else if (metric_type == "cosine")
                centroid_distance = vectors[centroid_i]->cosineDistance(vectors[i]);

            if (min_distance == -1 || centroid_distance < min_distance)
                min_distance = centroid_distance;
        }
    }

    return min_distance;
}


#endif //LIB_UTILS_H
