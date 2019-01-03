#ifndef CLUSTER_ASSIGNMENT_H
#define CLUSTER_ASSIGNMENT_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../data_structures/cust_vector.hpp"
#include "../data_structures/cust_hashtable.hpp"

#include "../lsh_cube.hpp"
#include "../utils.hpp"


/*
 * Functions used to implement various assignment algorithms required for vector clustering
 * Lloyd assignment, lsh and hypercube range searches are defined here
 *
 * All functions are templated, so they can be used for any kind of input vector type
 */


// Lloyd's assignment algorithm, for each input vector to be assigned, assign it to the cluster whose centroid is
// closest to it, depending on the metric used
template <typename vector_type>
void lloyds_assignment(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type);

// Lloyd's assignment as described above, but only assigns vectors that do dot belong to a cluster
template <typename vector_type>
void lloyds_for_remaining(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
                      std::string metric_type);


template <typename vector_type>
void lsh_range_assignment(std::vector< CustVector<vector_type> >& input_vectors,
        std::vector< CustHashtable<vector_type>* >& lsh_hashtables, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type);

template <typename vector_type>
void cube_range_assignment(std::vector< CustVector<vector_type> >& input_vectors,
        CustHashtable<vector_type>& hypercube, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type, int probes, int k);

template <typename vector_type>
void range_assignment(std::vector< std::vector< CustVector<vector_type>* > >& comb_buckets, std::vector< CustVector<vector_type>* >& centroids,
                      std::string metric_type);

/*
* Function definitions
*/

template <typename vector_type>
void lloyds_assignment(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type) {
    for (int vector_i = 0; vector_i < input_vectors.size(); vector_i++) {
        double min = -1;
        int min_centroid_i = 0;
        for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++) {
            double distance = 0;
            if (metric_type == "euclidean")
                distance = input_vectors[vector_i].euclideanDistance(centroids[centroid_i]);
            else if (metric_type == "cosine")
                distance = input_vectors[vector_i].cosineDistance(centroids[centroid_i]);

            if (min == -1 || distance < min) {
                min = distance;
                min_centroid_i = centroid_i;
            }
        }

        input_vectors[vector_i].setCluster(min_centroid_i, min);
    }

    // Each centroid is assigned to its cluster
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++)
        centroids[centroid_i]->setCluster(centroid_i, 0);

}


template <typename vector_type>
void lloyds_for_remaining(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
                       std::string metric_type) {
    for (int vector_i = 0; vector_i < input_vectors.size(); vector_i++) {
        if (input_vectors[vector_i].getCluster() == -1) {
            double min = -1;
            int min_centroid_i = 0;
            for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++) {
                double distance = 0;
                if (metric_type == "euclidean")
                    distance = input_vectors[vector_i].euclideanDistance(centroids[centroid_i]);
                else if (metric_type == "cosine")
                    distance = input_vectors[vector_i].cosineDistance(centroids[centroid_i]);

                if (min == -1 || distance < min) {
                    min = distance;
                    min_centroid_i = centroid_i;
                }
            }
            input_vectors[vector_i].setCluster(min_centroid_i, min);
        }
    }
}


template <typename vector_type>
void lsh_range_assignment(std::vector< CustVector<vector_type> >& input_vectors,
        std::vector< CustHashtable<vector_type>* >& lsh_hashtables, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type) {

    // For this algorithm, no vectors should be assigned to any cluster initially
    remove_clustering(input_vectors);

    // Get buckets from lsh hashtables
    std::vector< std::vector< CustVector<vector_type>* > > comb_buckets(centroids.size());
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++)
        comb_buckets[centroid_i] = get_LSH_combined_buckets<vector_type>(lsh_hashtables, centroids[centroid_i]);

    range_assignment(comb_buckets, centroids, metric_type);

    // Then, for use the standard lloyd's algorithm to assign any unassigned vectors to a centroid
    lloyds_for_remaining(input_vectors, centroids, metric_type);

    // Each centroid is assigned to its cluster
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++)
        centroids[centroid_i]->setCluster(centroid_i, 0);
}

template <typename vector_type>
void cube_range_assignment(std::vector< CustVector<vector_type> >& input_vectors,
                          CustHashtable<vector_type>& hypercube, std::vector< CustVector<vector_type>* >& centroids,
                          std::string metric_type, int probes, int k) {

    // For this algorithm, no vectors should be assigned to any cluster initially
    remove_clustering(input_vectors);

    // Get buckets from lsh hashtables
    std::vector< std::vector< CustVector<vector_type>* > > comb_buckets(centroids.size());
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++)
        comb_buckets[centroid_i] = get_hypercube_combined_buckets<vector_type>(hypercube, centroids[centroid_i], probes, k);

    range_assignment(comb_buckets, centroids, metric_type);

    // Then, for use the standard lloyd's algorithm to assign any unassigned vectors to a centroid
    lloyds_for_remaining(input_vectors, centroids, metric_type);

    // Each centroid is assigned to its cluster
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++)
        centroids[centroid_i]->setCluster(centroid_i, 0);
}


template <typename vector_type>
void range_assignment(std::vector< std::vector< CustVector<vector_type>* > >& comb_buckets, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type) {

    // First find the minimum distance between two centroids and make it divided by 2 the initial range for each range search
    // After each iteration the range will be doubled, until no new vectors are assigned
    double radius = find_min_vector_distance(centroids, metric_type) / 2;
    double min_radius = 0;
    // Use map to store calculated distances for this centroid as all distances are calculated for the first iteration anyway
    std::unordered_map<std::string, double> distanceMap;

    // For each centroid, search for R-Near neighbors with initial radius equal to input, then doubling it with
    // each iteration, until no new neighbors are found
    int assigned_count;
    int map_count = 0;
    do {
        assigned_count = 0;
        for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++) {
            std::vector< CustVector<vector_type>* > comb_bucket = comb_buckets[centroid_i];

            // For every vector in the combined lsh bucket, if that vector is not assigned and is inside the current radius
            // then assign it to current cluster
            // If it is assigned to another cluster, but is closer to the current one, reassign it
            for (auto bucketVector : comb_bucket) {

                if ( (bucketVector->getCluster() == -1) ||
                     (bucketVector->getCluster() != -1 && bucketVector->getDistFromCentroid() >= min_radius) ) {

                    std::string key = centroids[centroid_i]->getId() + "to" + bucketVector->getId();
                    double distance = 0;
                    if (distanceMap.count(key) == 1) {
                        distance = distanceMap[key];
                        map_count++;
                    } else {
                        if (metric_type == "euclidean")
                            distance = centroids[centroid_i]->euclideanDistance(bucketVector);
                        else if (metric_type == "cosine")
                            distance = centroids[centroid_i]->cosineDistance(bucketVector);
                        distanceMap[key] = distance;
                    }
                    // If vector is inside radius assign
                    if (distance >= min_radius && distance < radius) {
                        if (bucketVector->getCluster() == -1) {
                            bucketVector->setCluster(centroid_i, distance);
                            assigned_count++;
                        }
                        else {
                            if (bucketVector->getDistFromCentroid() > distance) {
                                bucketVector->setCluster(centroid_i, distance);
                                assigned_count++;
                            }
                        }
                    }
                }
            }

            min_radius = radius;
            radius = radius * 2;
        }

    }
    while (assigned_count > 0);
}



#endif //CLUSTER_ASSIGNMENT_H