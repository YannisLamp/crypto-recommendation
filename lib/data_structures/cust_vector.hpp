#ifndef LIB_CUST_VECTOR_H
#define LIB_CUST_VECTOR_H

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <set>

/*
 * Custom Vector
 *
 * Vector class used to represent vectors in this project
 * Contains its id and dimension values
 *
 * Operations between vectors are also implemented here (inner product, euclidean and cosine distances)
 *
 * Templated, so that it can have any dimension type
 */


template <typename dim_type>
class CustVector {
private:
    const std::string id;
    std::vector<dim_type> dimensions;

    // Indexes of which dimensions are cryptocurrencies that there is no opinion of
    // added for cryptocurrency recommendation functionality
    std::set<int> unknown_indexes;
    // Known cryptocurrency mean to use for normalization
    double known_mean;

    // Cluster information essential for clustering algorithms
    int cluster_i;
    double dist_from_centroid;

public:
    CustVector(std::string in_id, std::vector<dim_type> dim_vector);
    CustVector(std::string in_id, std::vector<dim_type> dim_vector, std::set<int> unknown_indexes, double mean);
    CustVector(std::string in_id, std::vector<dim_type> dim_vector, int cluster, double distance);

    // Vector operations
    template <typename in_dim_type>
    long double inner_product(CustVector<in_dim_type>* inVector, long double strt);
    template <typename in_dim_type>
    double euclideanDistance(CustVector<in_dim_type>* inVector);
    template <typename in_dim_type>
    double cosineDistance(CustVector<in_dim_type>* inVector);
    template <typename in_dim_type>
    double cosineSimilarity(CustVector<in_dim_type>* inVector);
    template <typename in_dim_type>
    void addVectorToThis(CustVector<in_dim_type>* inVector);
    void divDimensionsByD(double div_const);

    void setCluster(int index, double dist);
    void resetCluster();

    std::string getId();
    std::vector<dim_type>* getDimensions();
    std::vector<int> getUnknownIndexes();
    double getKnownMean();
    unsigned int getDimNumber();
    int getCluster();
    double getDistFromCentroid();
};


/*
* Template method definitions
*/


template <typename dim_type>
CustVector<dim_type>::CustVector(std::string in_id, std::vector<dim_type> dim_vector)
        : id(std::move(in_id)), dimensions(dim_vector), cluster_i(-1), dist_from_centroid(0), known_mean(0) {};

template <typename dim_type>
CustVector<dim_type>::CustVector(std::string in_id, std::vector<dim_type> dim_vector, std::set<int> indexes, double mean)
        : id(std::move(in_id)), dimensions(dim_vector), cluster_i(-1), dist_from_centroid(0), unknown_indexes(
        std::move(indexes)), known_mean(mean) {};

template <typename dim_type>
CustVector<dim_type>::CustVector(std::string in_id, std::vector<dim_type> dim_vector, int cluster, double distance)
        : id(std::move(in_id)), dimensions(dim_vector), cluster_i(cluster), dist_from_centroid(distance), known_mean(0) {};




template <typename dim_type>
template <typename in_dim_type>
long double CustVector<dim_type>::inner_product(CustVector<in_dim_type>* inVector, long double strt) {
    long double accum = strt;
    std::vector<in_dim_type>* in_dimensions = inVector->getDimensions();

    if (dimensions.size() != in_dimensions->size()) {
        std::cerr << id << " : Error in inner product with " << inVector->getId()
                  << ". Different number of dimensions" << std::endl;
        return -1;
    }

    for (unsigned int i = 0; i < dimensions.size(); i++)
        accum = accum + ( dimensions[i] * (*in_dimensions)[i] );

    return accum;
}


template <typename dim_type>
template <typename in_dim_type>
double CustVector<dim_type>::euclideanDistance(CustVector<in_dim_type>* inVector) {
    double dist = 0;
    double accum = 0;
    std::vector<in_dim_type>* in_dimensions = inVector->getDimensions();

    for (unsigned int i = 0; i < dimensions.size(); i++)
        accum = accum + pow( (dimensions[i] - (*in_dimensions)[i]) , 2);

    dist = sqrt(accum);
    return dist;
}


template <typename dim_type>
template <typename in_dim_type>
double CustVector<dim_type>::cosineDistance(CustVector<in_dim_type>* inVector) {
    long double inner_product = this->template inner_product<in_dim_type>(inVector, 0.0);

    // Calculate denominators
    std::vector<in_dim_type>* in_dimensions = inVector->getDimensions();
    double accum = 0;
    double in_accum = 0;
    for (unsigned int i = 0; i < dimensions.size(); i++) {
        accum = accum + pow(dimensions[i], 2);
        in_accum = in_accum + pow((*in_dimensions)[i], 2);
    }
    double denom = sqrt(accum) * sqrt(in_accum);

    return 1 - double( inner_product / denom );
}


template <typename dim_type>
template <typename in_dim_type>
double CustVector<dim_type>::cosineSimilarity(CustVector<in_dim_type>* inVector) {
    long double inner_product = this->template inner_product<in_dim_type>(inVector, 0.0);

    // Calculate denominators
    std::vector<in_dim_type>* in_dimensions = inVector->getDimensions();
    double accum = 0;
    double in_accum = 0;
    for (unsigned int i = 0; i < dimensions.size(); i++) {
        accum = accum + pow(dimensions[i], 2);
        in_accum = in_accum + pow((*in_dimensions)[i], 2);
    }
    double denom = sqrt(accum) * sqrt(in_accum);

    return double( inner_product / denom );
}


template <typename dim_type>
template <typename in_dim_type>
void CustVector<dim_type>::addVectorToThis(CustVector<in_dim_type>* inVector) {
    std::vector<in_dim_type> in_dimensions = *(inVector->getDimensions());

    for (int i = 0; i < dimensions.size(); i++)
        dimensions[i] = dimensions[i] + in_dimensions[i];
}


template <typename dim_type>
void CustVector<dim_type>::divDimensionsByD(double div_const) {
    if (div_const != 0) {
        for (int i = 0; i < dimensions.size(); i++)
            dimensions[i] = dimensions[i] / div_const;
    }

}


template <typename dim_type>
void CustVector<dim_type>::setCluster(int index, double dist) {
    cluster_i = index;
    dist_from_centroid = dist;
}


template <typename dim_type>
void CustVector<dim_type>::resetCluster() {
    cluster_i = -1;
    dist_from_centroid = 0;
}


template <typename dim_type>
std::string CustVector<dim_type>::getId() { return id; }


template <typename dim_type>
std::vector<dim_type>* CustVector<dim_type>::getDimensions() { return &dimensions; }


template <typename dim_type>
std::vector<int> CustVector<dim_type>::getUnknownIndexes() {
    std::vector<int> indexes_vector(unknown_indexes.begin(), unknown_indexes.end());
    return indexes_vector;
}

template <typename dim_type>
double CustVector<dim_type>::getKnownMean() { return known_mean; }


template <typename dim_type>
unsigned int CustVector<dim_type>::getDimNumber() { return dimensions.size(); }


template <typename dim_type>
int CustVector<dim_type>::getCluster() { return cluster_i; }


template <typename dim_type>
double CustVector<dim_type>::getDistFromCentroid() { return dist_from_centroid; }



#endif //LIB_CUST_VECTOR_H
