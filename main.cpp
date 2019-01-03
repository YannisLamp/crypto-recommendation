#include <iostream>
#include <string>

#include <chrono>
#include <random>

#include "./lib/in_out/arg_parser.h"
#include "./lib/in_out/vector_reader.hpp"
#include "./lib/data_structures/cust_vector.hpp"
#include "./lib/data_structures/cust_hashtable.hpp"
#include "./lib/lsh_cube.hpp"
#include "./lib/clustering_phases/initialization.hpp"
#include "./lib/clustering_phases/assignment.hpp"
#include "./lib/clustering_phases/update.hpp"
#include "./lib/clustering_phases/silhouette.hpp"

using namespace std;

void get_cluster_args(int argc, char* argv[], string* input_file, string* config_file, string* output_file,
                      string* metric_type, bool* print_complete);

void get_cluster_config(string config_file, int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w,
                        char* csv_delimiter, int* cube_range_c, int* cube_probes, int* max_algo_iterations, double* min_dist_kmeans);

template <typename vector_type>
void print_stats(std::ostream& os, std::vector< std::vector< CustVector<vector_type>* > > clusters,
                 std::vector< CustVector<vector_type>* >& centroids, std::vector<double>& sill, string algorithm, string metric_type,
                 chrono::duration<double> time_span, bool is_kmeans, bool print_complete);

int main(int argc, char* argv[]) {

    /*
     * Program Parameters
     * Read Input Data
     */

    // Get program options from arguments
    string input_file, config_file, output_file;
    string metric_type;
    bool print_complete = false;

    get_cluster_args(argc, argv, &input_file, &config_file, &output_file, &metric_type, &print_complete);

    // Get program options from configuration file
    int cluster_num = 0;
    int k = 4;
    int L = 5;
    int lsh_bucket_div = 4;
    double euclidean_h_w = 0.01;
    int cube_range_c = 1;
    int cube_probes = 0;
    int max_algo_iterations = 30;
    double min_dist_kmeans = 0.05;
    char csv_delimiter = ' ';

    get_cluster_config(config_file, &cluster_num, &k, &L, &lsh_bucket_div, &euclidean_h_w, &csv_delimiter,
            &cube_range_c, &cube_probes, &max_algo_iterations, &min_dist_kmeans);

    // Read and save vectors from specified input file, parse metric option
    VectorReader<double>* inputReader = new VectorReader<double>(input_file);
    if ( !inputReader->read(csv_delimiter, 1, [](const string& x){ return stod(x); }) ) {
        std::cerr << "Error opening file " + input_file << std::endl;
        return -1;
    }
    vector< CustVector<double> > input_vectors = inputReader->getReadVectors();
    if (input_vectors.empty()) {
        return -1;
    }
    delete inputReader;


    /*
     * Input Data Prepossessing
     * Create and Populate Hashtables for LSH and Hypercube
     */

    vector< CustHashtable<double>* > lsh_hashtables = create_LSH_hashtables<double>(input_vectors, metric_type, k, L,
            lsh_bucket_div, euclidean_h_w);

    CustHashtable<double>* hypercube = create_hypercube<double>(input_vectors, metric_type, k, euclidean_h_w);

    /*
     * Main Clustering Algorithms
     * Various combinations of clustering phases
     * Time algorithms evaluate them and generate stats
     */

    // Open output file
    ofstream outFile(output_file);

    // First combination I1A1U1
    {
        string algorithm = "I1A1U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }


    // Second combination I1A1U2
    {
        string algorithm = "I1A1U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);

    }


    // Third combination I1A2U1
    {
        string algorithm = "I1A2U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lsh_range_assignment(input_vectors, lsh_hashtables, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }

    // Forth combination I1A2U2
    {
        string algorithm = "I1A2U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lsh_range_assignment(input_vectors, lsh_hashtables, centroids, metric_type);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);
    }

    // Fifth combination I1A3U1
    {
        string algorithm = "I1A3U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            cube_range_assignment(input_vectors, *hypercube, centroids, metric_type, cube_probes, k);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }

    // Sixth combination I1A3U2
    {
        string algorithm = "I1A3U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            cube_range_assignment(input_vectors, *hypercube, centroids, metric_type, cube_probes, k);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);

    }

    // Seventh combination I2A1U1
    {
        string algorithm = "I2A1U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }


    // Eighth combination I2A1U2
    {
        string algorithm = "I2A1U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);

    }


    // Ninth combination I2A2U1
    {
        string algorithm = "I2A2U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lsh_range_assignment(input_vectors, lsh_hashtables, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }

    // Tenth combination I2A2U2
    {
        string algorithm = "I2A2U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lsh_range_assignment(input_vectors, lsh_hashtables, centroids, metric_type);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);
    }

    // Eleventh combination I2A3U1
    {
        string algorithm = "I2A3U1";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            cube_range_assignment(input_vectors, *hypercube, centroids, metric_type, cube_probes, k);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }

    // Twelfth combination I2A3U2
    {
        string algorithm = "I2A3U2";
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            cube_range_assignment(input_vectors, *hypercube, centroids, metric_type, cube_probes, k);
            continue_clustering = pam_lloyds(input_vectors, centroids, metric_type);
            clustering_iterations++;
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(t2 - t1);

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, false, print_complete);
    }

    /*
     * Program End
     * Free Allocated Memory
     */

    outFile.close();
    for (int i = 0; i < lsh_hashtables.size(); i++) {
        delete lsh_hashtables[i];
    }
    delete hypercube;
}


void get_cluster_args(int argc, char* argv[], string* input_file, string* config_file, string* output_file,
        string* metric_type, bool* print_complete) {
    ArgParser* progArgs = new ArgParser(argc, argv);

    // For file paths, if no argument is given, request it from the user
    if (progArgs->flagExists("-i"))
        *input_file = progArgs->getFlagValue("-i");
    else {
        cout << "Please specify input file path" << endl;
        cin >> *input_file;
    }
    if (progArgs->flagExists("-c"))
        *config_file = progArgs->getFlagValue("-c");
    else {
        cout << "Please specify configuration file path" << endl;
        cin >> *config_file;
    }
    if (progArgs->flagExists("-o"))
        *output_file = progArgs->getFlagValue("-o");
    else {
        cout << "Please specify output file path" << endl;
        cin >> *output_file;
    }
    if (progArgs->flagExists("-d"))
        *metric_type = progArgs->getFlagValue("-d");
    else {
        cout << "Please specify metric to be used" << endl;
        cin >> *metric_type;
    }
    if (progArgs->flagExists("-complete"))
        *print_complete = true;

    delete progArgs;
}


void get_cluster_config(string config_file, int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w,
        char* csv_delimiter, int* cube_range_c, int* cube_probes, int* max_algo_iterations, double* min_dist_kmeans) {

    ArgParser* configArgs = new ArgParser( file_to_args(config_file, ' ') );

    if (configArgs->flagExists("number_of_clusters:"))
        *cluster_num = stoi( configArgs->getFlagValue("number_of_clusters:") );
    else {
        cout << "Please specify the number of clusters to be found" << endl;
        cin >> *cluster_num;
    }
    if (configArgs->flagExists("number_of_hash_functions:"))
        *k = stoi( configArgs->getFlagValue("number_of_hash_functions:") );
    if (configArgs->flagExists("number_of_hash_tables:"))
        *L = stoi( configArgs->getFlagValue("number_of_hash_tables:") );
    if (configArgs->flagExists("lsh_bucket_div:"))
        *lsh_bucket_div = stoi( configArgs->getFlagValue("lsh_bucket_div:") );
    if (configArgs->flagExists("euclidean_h_w:"))
        *euclidean_h_w = stod( configArgs->getFlagValue("euclidean_h_w:") );
    if (configArgs->flagExists("cube_range_c:"))
        *cube_range_c = stoi( configArgs->getFlagValue("cube_range_c:") );
    if (configArgs->flagExists("cube_probes:"))
        *cube_probes = stoi( configArgs->getFlagValue("cube_probes:") );
    if (configArgs->flagExists("max_algo_iterations:"))
        *max_algo_iterations = stoi( configArgs->getFlagValue("max_algo_iterations:") );
    if (configArgs->flagExists("min_dist_kmeans:"))
        *min_dist_kmeans = stod( configArgs->getFlagValue("min_dist_kmeans:") );
    if (configArgs->flagExists("csv_delimiter:")) {
        string delim_str = configArgs->getFlagValue("csv_delimiter:");
        *csv_delimiter = delim_str[1];
    }

    delete configArgs;
}


// Print stats
template <typename vector_type>
void print_stats(std::ostream& os, std::vector< std::vector< CustVector<vector_type>* > > clusters,
        std::vector< CustVector<vector_type>* >& centroids, std::vector<double>& sill, string algorithm, string metric_type,
        chrono::duration<double> time_span, bool is_kmeans, bool print_complete) {
    os << "Algorithm: " << algorithm << "\n";
    os << "Metric: " << metric_type << "\n";
    for (int cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
        os << "CLUSTER-" << cluster_i + 1 << " {size: " << clusters[cluster_i].size() << " centroid: ";
        if (is_kmeans) {
            std::vector<vector_type>* dims = centroids[cluster_i]->getDimensions();
            for (int i = 0; i < dims->size(); i++)
                os << (*dims)[i] << " ";
            os << "}" << endl;
        }
        else
            os << centroids[cluster_i]->getId() << "}" << endl;
    }
    os << "clustering_time: " << time_span.count() << endl;
    os << "Silhouette: [";
    os << sill[0];
    for (int i = 1; i < sill.size(); i++)
        os << ", " << sill[i];
    os << "]" << endl;

    if (print_complete) {
        for (int cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
            os << "CLUSTER-" << cluster_i + 1 << " {";
            os << clusters[cluster_i][0]->getId();
            for (int i = 1; i < clusters[cluster_i].size(); i++)
                os << ", " << clusters[cluster_i][i]->getId();
            os << "}" << endl;
        }
    }
}


