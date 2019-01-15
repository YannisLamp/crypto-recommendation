#include <iostream>
#include <string>

#include <chrono>
#include <random>
#include <vector>
#include <utility>

#include "./lib/in_out/arg_parser.h"
#include "./lib/in_out/vector_reader.hpp"
#include "./lib/data_structures/cust_vector.hpp"
#include "./lib/data_structures/cust_hashtable.hpp"
#include "./lib/data_structures/tweet.h"
#include "./lib/lsh_cube.hpp"
#include "./lib/clustering_phases/initialization.hpp"
#include "./lib/clustering_phases/assignment.hpp"
#include "./lib/clustering_phases/update.hpp"
#include "./lib/clustering_phases/silhouette.hpp"
#include "./lib/crypto_rec.hpp"

using namespace std;

void get_recommendation_args(int argc, char* argv[], string* input_file, string* output_file, bool* validate);

void get_config(string config_file, int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w,
                        char* csv_delimiter, int* cube_range_c, int* cube_probes, int* max_algo_iterations,
                        double* min_dist_kmeans, string* lexicon_file, string* query_file);

void print_recommendations(std::ostream& os, string user_id, vector<int> recom_crypto_indexes,
        vector< vector<string> > query_crypto, int name_index);

int main(int argc, char* argv[]) {

    /*
     * Program Parameters
     * Read Input Data
     */

    // Get program options from arguments
    string input_file, config_file, output_file;
    bool validate = "false";

    get_recommendation_args(argc, argv, &input_file, &output_file, &validate);
    config_file = "./cluster.conf";

    // Get all necessary program options from configuration file, even configurations for assignment 2 clustering
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
    string lexicon_file, query_file;

    get_config(config_file, &cluster_num, &k, &L, &lsh_bucket_div, &euclidean_h_w, &csv_delimiter, &cube_range_c,
            &cube_probes, &max_algo_iterations, &min_dist_kmeans, &lexicon_file, &query_file);


    /*
     * Create assignment 2 clustering data
     *
     * Note that any combination of clustering phases from assignment 2 can be used here, though a combination of
     * kmeans++ for initialization, lloyds for assignment and kmeans for updating the cluster centers is
     * used here, as it provides a very good results quite fast
     *
     * To speed up the clustering, random center selection can be used instead of kmeans++
     */


    // Read and save vectors from specified input file, parse metric option
    /*VectorReader<double>* inputReader = new VectorReader<double>(input_file);
    if ( !inputReader->read(csv_delimiter, 1, [](const string& x){ return stod(x); }) ) {
        std::cerr << "Error opening file " + input_file << std::endl;
        return -1;
    }
    vector< CustVector<double> > input_vectors_of_2 = inputReader->getReadVectors();
    if (input_vectors_of_2.empty()) {
        return -1;
    }
    delete inputReader;



    // Fast and reliable clustering, add random selection to make it faster
    {
        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        //vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }




    */


    /*
     * Input Data Prepossessing
     * Create and Populate Hashtables for LSH and Hypercube
     */

    //vector< CustHashtable<double>* > lsh_hashtables = create_LSH_hashtables<double>(input_vectors, metric_type, k, L,
    //        lsh_bucket_div, euclidean_h_w);

    int P = 1;
    vector< vector<string> > input_tweets = file_to_str_vectors(input_file, csv_delimiter, &P);
    vector< vector<string> > query_crypto = file_to_str_vectors(query_file, csv_delimiter);
    unordered_map<string, float> lexicon = file_to_lexicon(lexicon_file, csv_delimiter);



    // Create tweet unordered map
    unordered_map<string, Tweet> tweets;
    for (auto& tweet_words : input_tweets) {
        Tweet tweetWStats(tweet_words, lexicon, query_crypto);
        tweets.emplace(tweetWStats.getId(), tweetWStats);
    }

    // Convert tweets to user vectors, also filter useless users and give the unknown rating the value of the vector's mean
    vector< CustVector<double> > user_vectors = tweets_to_user_vectors<double>(tweets, query_crypto.size());
    ofstream outFile(output_file);

    /*
     * Cosine LSH Recommendation
     *
     * Part A.
     */


    // Create LSH hashtables for LSH recommendation
    {
        string metric_type = "cosine";
        outFile << "Cosine LSH" << endl;
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
        vector<CustHashtable<double>*> lsh_hashtables = create_LSH_hashtables<double>(user_vectors, metric_type, k, L,
                lsh_bucket_div, euclidean_h_w);

        // For each user, calculate actual recommendations
        for (auto &user : user_vectors) {
            std::vector< CustVector<double>* > neighbors = get_LSH_filtered_combined_buckets(lsh_hashtables, &user);
            vector<double> similarities = get_P_closest(neighbors, user, P);

            // Get top 5 recommendations
            // Note that the user vectors have not been normalized yet, only the unknown cryptocurrency values have the
            // mean value. So during this proccess the mean of the vector is subtracted from each rating
            vector<int> recom_crypto_indexes = get_top_N_recom(neighbors, user, 5, similarities);
            print_recommendations(outFile, user.getId(), recom_crypto_indexes, query_crypto, 4);
        }
        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast<chrono::milliseconds>(t2 - t1);
        outFile << "Execution Time: " << time_span.count() << endl;

        for (int i = 0; i < lsh_hashtables.size(); i++)
            delete lsh_hashtables[i];
    }
    int aaa=0;


    /*
     * Cosine LSH Recommendation
     *
     * Part B.
     */



    /*
     * Clustering Recommendation
     *
     * Part A.
     */


    // Fast and reliable clustering, add random selection to make it faster
    /*{
        vector<CustVector<double> *> centroids = rand_selection(input_vectors, cluster_num);
        //vector<CustVector<double> *> centroids = k_means_pp(input_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors, centroids, metric_type);
            continue_clustering = k_means(input_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        // Stats printing
        std::vector<std::vector<CustVector<double> *> > clusters = separate_clusters_from_input(input_vectors,
                                                                                                centroids.size());
        std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        print_stats<double>(outFile, clusters, centroids, sill, algorithm, metric_type, time_span, true, print_complete);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];

    }*/





    /*
     * Program End
     * Free Allocated Memory
     */

    outFile.close();
    /*for (int i = 0; i < lsh_hashtables.size(); i++) {
        delete lsh_hashtables[i];
    }


     for (auto& user : user_vectors) {
        if (user.getId() == "30")
            int aaaa=0;
    }
    */
}


void get_recommendation_args(int argc, char* argv[], string* input_file, string* output_file, bool* validate) {
    ArgParser* progArgs = new ArgParser(argc, argv);

    // For file paths, if no argument is given, request it from the user
    if (progArgs->flagExists("-d"))
        *input_file = progArgs->getFlagValue("-d");
    else {
        cout << "Please specify input file path" << endl;
        cin >> *input_file;
    }
    if (progArgs->flagExists("-o"))
        *output_file = progArgs->getFlagValue("-o");
    else {
        cout << "Please specify output file path" << endl;
        cin >> *output_file;
    }
    if (progArgs->flagExists("-validate"))
        *validate = true;

    delete progArgs;
}


void get_config(string config_file, int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w,
        char* csv_delimiter, int* cube_range_c, int* cube_probes, int* max_algo_iterations, double* min_dist_kmeans,
        string* lexicon_file, string* query_file) {

    ArgParser* configArgs = new ArgParser( file_to_args(config_file, ' ') );

    if (configArgs->flagExists("number_of_clusters"))
        *cluster_num = stoi( configArgs->getFlagValue("number_of_clusters") );
    else {
        cout << "Please specify the number of clusters to be found" << endl;
        cin >> *cluster_num;
    }
    if (configArgs->flagExists("number_of_hash_functions"))
        *k = stoi( configArgs->getFlagValue("number_of_hash_functions") );
    if (configArgs->flagExists("number_of_hash_tables"))
        *L = stoi( configArgs->getFlagValue("number_of_hash_tables") );
    if (configArgs->flagExists("lsh_bucket_div"))
        *lsh_bucket_div = stoi( configArgs->getFlagValue("lsh_bucket_div") );
    if (configArgs->flagExists("euclidean_h_w"))
        *euclidean_h_w = stod( configArgs->getFlagValue("euclidean_h_w") );
    if (configArgs->flagExists("cube_range_c"))
        *cube_range_c = stoi( configArgs->getFlagValue("cube_range_c") );
    if (configArgs->flagExists("cube_probes"))
        *cube_probes = stoi( configArgs->getFlagValue("cube_probes") );
    if (configArgs->flagExists("max_algo_iterations"))
        *max_algo_iterations = stoi( configArgs->getFlagValue("max_algo_iterations") );
    if (configArgs->flagExists("min_dist_kmeans"))
        *min_dist_kmeans = stod( configArgs->getFlagValue("min_dist_kmeans") );
    if (configArgs->flagExists("csv_delimiter")) {
        string delim_str = configArgs->getFlagValue("csv_delimiter");
        *csv_delimiter = char( stoi(delim_str) );
    }
    if (configArgs->flagExists("lexicon_file"))
        *lexicon_file = configArgs->getFlagValue("lexicon_file");
    if (configArgs->flagExists("query_file"))
        *query_file = configArgs->getFlagValue("query_file");

    delete configArgs;
}


void print_recommendations(std::ostream& os, string user_id, vector<int> recom_crypto_indexes,
        vector< vector<string> > query_crypto, int name_index) {
    os << user_id;

    // Print the name of the recommended cryptocurrency
    for (int index : recom_crypto_indexes) {
        if (query_crypto[index].size() > name_index)
            os << " " << query_crypto[index][name_index];
        else
            os << " " << query_crypto[index][0];
    }
    os << "\n";
}
