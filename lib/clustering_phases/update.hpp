#ifndef CLUSTER_UPDATE_H
#define CLUSTER_UPDATE_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../data_structures/cust_vector.hpp"

/*
 * Functions used to implement various update algorithms required for vector clustering
 *
 * All functions are templated, so they can be used for any kind of input vector type
 */

// K_means update algorithm, for each cluster, calculates the mean of all its members and creates vectors found dimensions
// If clustering should stop (same centers are found, or previous and new centers do not exceed a minimum distance)
// return false, otherwise true
// Important, after clustering algorithm ends, the centers that are created by this function need to be deleted
template <typename vector_type>
bool k_means(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type, double min_dist);

// Pam algorithm improved like Lloyd's, for each cluster, calculate the median that minimizes the distance of all other
// vectors of the cluster and it, then switch the current centroid with it
// If clustering should stop (same centroids are found) return false, otherwise true
template <typename vector_type>
bool pam_lloyds(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
        std::string metric_type);


/*
* Function definitions
*/

template <typename vector_type>
bool k_means(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centers,
        std::string metric_type, double min_dist) {

    // Vector centers to return after calculations (they are not returned, rather they replace the current input centers)
    std::vector< CustVector<vector_type>* > new_centers(centers.size());
    for (int cluster_i = 0; cluster_i < new_centers.size(); cluster_i++) {
        std::vector<vector_type> i_center_dims(centers[cluster_i]->getDimNumber());

        for (int i = 0; i < i_center_dims.size(); i++)
            i_center_dims[i] = 0;
        new_centers[cluster_i] = new CustVector<vector_type>("k_means_center", i_center_dims);
    }

    // For each input vector, calculate the center of the cluster that it belongs to
    std::vector<int> cluster_member_num(centers.size());
    for (auto in_vector : input_vectors) {
        cluster_member_num[in_vector.getCluster()]++;
        new_centers[in_vector.getCluster()]->template addVectorToThis<vector_type>(&in_vector);
    }
    for (int cluster_i = 0; cluster_i < new_centers.size(); cluster_i++)
        new_centers[cluster_i]->divDimensionsByD(cluster_member_num[cluster_i]);

    // After calculating the new centers, determine if clustering should stop, with no new centers returned
    // If centers are the same as previously, or the the mean distance between new and old centers is below an input
    // min_dist, then return false
    for (int cluster_i = 0; cluster_i < new_centers.size(); cluster_i++) {
        double distance = 0;
        if (metric_type == "euclidean")
            distance = new_centers[cluster_i]->euclideanDistance(centers[cluster_i]);
        else if (metric_type == "cosine")
            distance = new_centers[cluster_i]->cosineDistance(centers[cluster_i]);

        // If at least one center passes the check, reassign all centers
        if (distance > min_dist) {
            for (int i = 0; i < centers.size(); i++) {
                if (centers[i]->getId() == "k_means_center")
                    delete centers[i];
                centers[i] = new_centers[i];
            }
            // Continue clustering
            return true;
        }
    }

    // Else free created centers and stop clustering process
    for (int i = 0; i < new_centers.size(); i++)
        delete new_centers[i];
    return false;
}


template <typename vector_type>
bool pam_lloyds(std::vector< CustVector<vector_type> >& input_vectors, std::vector< CustVector<vector_type>* >& centroids,
                std::string metric_type) {

    // Get different clusters from input separated
    std::vector< std::vector< CustVector<vector_type>* > > clusters = separate_clusters_from_input(input_vectors, centroids.size());

    bool median_swapped = false;
    // For each cluster calculate median and swap
    for (int cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
        double min_dist_sum = -1;
        int min_dist_i = 0;
        // Dictionary to cache distances
        std::unordered_map<std::string, double> distanceMap;
        for (int pot_median_i = 0; pot_median_i < clusters[cluster_i].size(); pot_median_i++) {
            double dist_sum = 0;
            for (int curr_dist_i = 0; curr_dist_i < clusters[cluster_i].size(); curr_dist_i++) {
                double distance = 0;

                std::string key = clusters[cluster_i][pot_median_i]->getId() + "to" + clusters[cluster_i][curr_dist_i]->getId();
                std::string reverse_key = clusters[cluster_i][curr_dist_i]->getId() + "to" + clusters[cluster_i][pot_median_i]->getId();
                if (distanceMap.count(key) == 1) {
                    distance = distanceMap[key];
                }
                // Else calculate distance and save it in the map
                else {
                    if (metric_type == "euclidean")
                        distance = clusters[cluster_i][pot_median_i]->euclideanDistance(clusters[cluster_i][curr_dist_i]);
                    else if (metric_type == "cosine")
                        distance = clusters[cluster_i][pot_median_i]->cosineDistance(clusters[cluster_i][curr_dist_i]);

                    distanceMap[reverse_key] = distance;
                }
                dist_sum = dist_sum + distance;
            }

            if (min_dist_sum == -1 || dist_sum < min_dist_sum) {
                min_dist_sum = dist_sum;
                min_dist_i = pot_median_i;
            }
        }

        // If the found median and the previous centroid are not the same vector, then swap them
        if (clusters[cluster_i][min_dist_i]->getId() != centroids[cluster_i]->getId()) {
            centroids[cluster_i] = clusters[cluster_i][min_dist_i];
            median_swapped = true;
        }
    }

    if (median_swapped)
        return true;
    else
        return false;
}

#endif //CLUSTER_UPDATE_H