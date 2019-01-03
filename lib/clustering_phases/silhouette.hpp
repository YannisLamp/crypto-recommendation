#ifndef SILHOUETTE_H
#define SILHOUETTE_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../data_structures/cust_vector.hpp"
#include "../data_structures/cust_hashtable.hpp"

#include "../lsh_cube.hpp"

/*
 * Silhouette function, used to evaluate clusters
 * Templated, so it can be used for any kind of input vector type
 */

template <typename vector_type>
std::vector<double> silhouette_cluster(std::vector< std::vector< CustVector<vector_type>* > > clusters, std::vector< CustVector<vector_type>* >& centroids,
                  std::string metric_type);

template <typename vector_type>
double silhouette_of_i(std::vector< CustVector<vector_type>* >& cluster, int sil_vector_i,
        std::vector< CustVector<vector_type>* >& neighbor_cluster, std::string metric_type, std::unordered_map<std::string, double>& distanceMap);

/*
* Function definitions
*/

template <typename vector_type>
std::vector<double> silhouette_cluster(std::vector< std::vector< CustVector<vector_type>* > > clusters, std::vector< CustVector<vector_type>* >& centroids,
                std::string metric_type) {

    // First, for each centroid, find its nearest
    std::vector<int> near_centroid_i(centroids.size());
    for (int centroid_i = 0; centroid_i < centroids.size(); centroid_i++) {
        double min_distance = -1;
        int min_centroid_i = 0;
        for (int i = 0; i < centroids.size(); i++) {

            if (i != centroid_i) {
                double distance = 0;
                if (metric_type == "euclidean")
                    distance = centroids[centroid_i]->euclideanDistance(centroids[i]);
                else if (metric_type == "cosine")
                    distance = centroids[centroid_i]->cosineDistance(centroids[i]);

                if (min_distance == -1 || distance < min_distance) {
                    min_distance = distance;
                    min_centroid_i = i;
                }
            }
        }

        // Update nearest centroid index
        near_centroid_i[centroid_i] = min_centroid_i;
    }

    // Use map to cache calculated distances (all calculated distances are very likely to be used a lot more than once,
    // even the distances between neighboring cluster vectors)
    std::unordered_map<std::string, double> distanceMap;

    std::vector<double> sils(clusters.size()+1);
    sils[clusters.size()] = 0;
    int vector_num = 0;

    for (int cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
        sils[cluster_i] = 0;
        for (int vec_i = 0; vec_i < clusters[cluster_i].size(); vec_i++)
            sils[cluster_i] = sils[cluster_i] + silhouette_of_i(clusters[cluster_i], vec_i, clusters[near_centroid_i[cluster_i]],
                    metric_type, distanceMap);
        sils[clusters.size()] = sils[clusters.size()] + sils[cluster_i];
        sils[cluster_i] = sils[cluster_i] / clusters[cluster_i].size();

        vector_num = vector_num + clusters[cluster_i].size();
    }
    sils[clusters.size()] = sils[clusters.size()] / vector_num;

    return sils;
}


template <typename vector_type>
double silhouette_of_i(std::vector< CustVector<vector_type>* >& cluster, int sil_vector_i,
        std::vector< CustVector<vector_type>* >& neighbor_cluster, std::string metric_type,
        std::unordered_map<std::string, double>& distanceMap) {

    // Calculate a(i)
    double a_i = 0;
    for (int cluster_i = 0; cluster_i < cluster.size(); cluster_i++) {
        std::string key = cluster[sil_vector_i]->getId() + "to" + cluster[cluster_i]->getId();
        std::string reverse_key = cluster[cluster_i]->getId() + "to" + cluster[sil_vector_i]->getId();

        double distance = 0;
        if (distanceMap.count(key) == 1) {
            distance = distanceMap[key];
        }
        // If distance is not in the map, calculate and save it (with the reverse key, as it will not be needed a
        // second time for this particular vector
        else {
            if (metric_type == "euclidean")
                distance = cluster[sil_vector_i]->euclideanDistance(cluster[cluster_i]);
            else if (metric_type == "cosine")
                distance = cluster[sil_vector_i]->cosineDistance(cluster[cluster_i]);
            distanceMap[reverse_key] = distance;
        }

        a_i = a_i + distance;
    }
    if (cluster.size() != 1)
        a_i = a_i / (cluster.size()-1);

    // Calculate b(i)
    double b_i = 0;
    for (int neig_cluster_i = 0; neig_cluster_i < neighbor_cluster.size(); neig_cluster_i++) {
        std::string key = cluster[sil_vector_i]->getId() + "to" + neighbor_cluster[neig_cluster_i]->getId();
        std::string reverse_key = neighbor_cluster[neig_cluster_i]->getId() + "to" + cluster[sil_vector_i]->getId();

        double distance = 0;
        if (distanceMap.count(key) == 1) {
            distance = distanceMap[key];
        }
        // If distance is not in the map, calculate and save it (with the reverse key, as it will not be needed a
        // second time for this particular vector
        else {
            if (metric_type == "euclidean")
                distance = cluster[sil_vector_i]->euclideanDistance(neighbor_cluster[neig_cluster_i]);
            else if (metric_type == "cosine")
                distance = cluster[sil_vector_i]->cosineDistance(neighbor_cluster[neig_cluster_i]);
            distanceMap[reverse_key] = distance;
        }

        b_i = b_i + distance;
    }
    b_i = b_i / neighbor_cluster.size();

    // Calculate and return s(i)
    double max_i = a_i;
    if (b_i > a_i)
        max_i = b_i;

    return (b_i - a_i) / max_i;
}








#endif //SILHOUETTE_H