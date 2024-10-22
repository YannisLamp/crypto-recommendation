#include <iostream>
#include <string>

#include <chrono>
#include <random>
#include <vector>
#include <utility>
#include <cmath>

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

double lsh_rec_10_fold_validation_A(vector< CustVector<double> >& user_vectors, string metric_type, int k, int L,
        int lsh_bucket_div, double euclidean_h_w, int P);

void get_recommendation_args(int argc, char* argv[], string* input_file, string* output_file, bool* validate);

void get_config(string config_file, string* proj_2_input, char* proj_2_csv_delimiter, int* proj_2_cluster_num,
                int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w, char* csv_delimiter,
                int* max_algo_iterations, double* min_dist_kmeans, string* lexicon_file, string* query_file);

void print_recommendations(std::ostream& os, string user_id, vector<int> recom_crypto_indexes,
        vector< vector<string> > query_crypto, int name_index);

int main(int argc, char* argv[]) {

    /*
     * Program Parameters
     * Read Input Data
     */

    // Get program options from arguments
    string input_file, config_file, output_file;
    bool validate = false;

    get_recommendation_args(argc, argv, &input_file, &output_file, &validate);
    config_file = "./cluster.conf";

    // Get all necessary program options from configuration file, even configurations for assignment 2 clustering
    string proj_2_input;
    char proj_2_csv_delimiter = ' ';
    int  proj_2_cluster_num = 100;

    int cluster_num = 0;
    int k = 4;
    int L = 5;
    int lsh_bucket_div = 4;
    double euclidean_h_w = 0.01;
    int max_algo_iterations = 30;
    double min_dist_kmeans = 0.05;
    char csv_delimiter = ' ';
    string lexicon_file, query_file;

    get_config(config_file, &proj_2_input, &proj_2_csv_delimiter, &proj_2_cluster_num, &cluster_num, &k, &L,
            &lsh_bucket_div, &euclidean_h_w, &csv_delimiter, &max_algo_iterations, &min_dist_kmeans, &lexicon_file, &query_file);


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
    VectorReader<double>* inputReader = new VectorReader<double>(proj_2_input);
    if ( !inputReader->read(proj_2_csv_delimiter, 1, [](const string& x){ return stod(x); }) ) {
        std::cerr << "Error opening file " + input_file << std::endl;
        return -1;
    }
    vector< CustVector<double> > input_vectors_of_2 = inputReader->getReadVectors();
    if (input_vectors_of_2.empty()) {
        return -1;
    }
    delete inputReader;

    //std::vector<std::vector<CustVector<double> *> > clusters_of_2;
    // Fast and accurate clustering
    {
        string metric_type = "cosine";
        vector<CustVector<double> *> centroids = k_means_pp(input_vectors_of_2, proj_2_cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(input_vectors_of_2, centroids, metric_type);
            continue_clustering = k_means(input_vectors_of_2, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }

        //clusters_of_2 = separate_clusters_from_input(input_vectors_of_2, centroids.size());
        //std::vector<double> sill = silhouette_cluster(clusters_of_2, centroids, metric_type);

        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];
    }


    /*
     * Input Data Prepossessing
     * Create and Populate Hashtables for LSH and Hypercube
     */


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
    vector< CustVector<double> > fake_user_vectors = clusters_to_user_vectors(tweets, input_vectors_of_2,
            query_crypto.size(), proj_2_cluster_num);

    //input_vectors_of_2.resize(0);
    ofstream outFile(output_file);

    /*
     * Cosine LSH Recommendation
     *
     * Part A.
     */


    {
        string metric_type = "cosine";
        outFile << "Cosine LSH" << endl;
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Create LSH hashtables for LSH recommendation
        vector<CustHashtable<double>*> lsh_hashtables = create_LSH_hashtables<double>(user_vectors, metric_type, k, L,
                lsh_bucket_div, euclidean_h_w);

        // For each user, calculate actual recommendations
        for (auto &user : user_vectors) {
            std::vector< CustVector<double>* > neighbors = get_LSH_filtered_combined_buckets(lsh_hashtables, &user);
            if (!neighbors.empty()) {
                vector<double> similarities1 = get_P_closest(neighbors, user, P);

                // Get top 5 recommendations
                // Note that the user vectors have not been normalized yet, only the unknown cryptocurrency values have the
                // mean value. So during this proccess the mean of the vector is subtracted from each rating
                vector<int> recom_crypto_indexes1 = get_top_N_recom(neighbors, user, 5, similarities1);
                print_recommendations(outFile, user.getId(), recom_crypto_indexes1, query_crypto, 4);
            }
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        outFile << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << endl;

        for (int i = 0; i < lsh_hashtables.size(); i++)
            delete lsh_hashtables[i];


        // 10-fold cross-validation
        if (validate) {
            double validation = lsh_rec_10_fold_validation_A(user_vectors, metric_type, k, L, lsh_bucket_div, euclidean_h_w, P);
            cout << " aa" << validation << endl;
        }

    }


    /*
     * Cosine LSH Recommendation
     *
     * Part B.
     */


    {
        string metric_type = "cosine";
        outFile << "Cosine LSH" << endl;
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Create LSH hashtables for LSH recommendation
        vector<CustHashtable<double>*> lsh_hashtables = create_LSH_hashtables<double>(fake_user_vectors, metric_type, k, L,
                lsh_bucket_div, euclidean_h_w);

        // For each user, calculate actual recommendations
        for (auto &user : user_vectors) {
            std::vector< CustVector<double>* > neighbors = get_LSH_filtered_combined_buckets(lsh_hashtables, &user);
            if (!neighbors.empty()) {
                vector<double> similarities = get_P_closest(neighbors, user, P);

                // Get top 2 recommendations
                // Note that the user vectors have not been normalized yet, only the unknown cryptocurrency values have the
                // mean value. So during this proccess the mean of the vector is subtracted from each rating
                vector<int> recom_crypto_indexes = get_top_N_recom(neighbors, user, 2, similarities);
                print_recommendations(outFile, user.getId(), recom_crypto_indexes, query_crypto, 4);
            }
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        outFile << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << endl;

        for (int i = 0; i < lsh_hashtables.size(); i++)
            delete lsh_hashtables[i];


        // 10-fold cross-validation
        //if (validate) {
        //    lsh_rec_10_fold_validation_B(user_vectors, fake_user_vectors, metric_type, k, L, lsh_bucket_div, euclidean_h_w, P);
        //}

    }


    /*
     * Clustering Recommendation
     *
     * Part A.
     */


    {
        string metric_type = "euclidean";
        outFile << "Clustering Recommendation" << endl;
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Begin clustering
        vector<CustVector<double> *> centroids = rand_selection(user_vectors, cluster_num);
        //vector<CustVector<double> *> centroids = k_means_pp(user_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(user_vectors, centroids, metric_type);
            continue_clustering = k_means(user_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }
        std::vector< std::vector<CustVector<double>*> > clusters = separate_clusters_from_input(user_vectors,
                centroids.size());
        //std::vector<double> sill = silhouette_cluster(user_vectors, centroids, metric_type);

        // Begin calculating optimal recommendations
        for (auto &user : user_vectors) {
            std::vector< CustVector<double>* > neighbors = clusters[user.getCluster()];
            if (!neighbors.empty()) {
                // Get top 5 recommendations
                // Note that the user vectors have not been normalized yet, only the unknown cryptocurrency values have the
                // mean value. So during this proccess the mean of the vector is subtracted from each rating
                vector<int> recom_crypto_indexes = get_top_N_recom(neighbors, user, 5);
                print_recommendations(outFile, user.getId(), recom_crypto_indexes, query_crypto, 4);
            }
        }

        //std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        outFile << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << endl;
        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];


        // 10-fold cross-validation
        /*if (validate) {
            vector< CustVector<double> > temp_user_vectors = fake_user_vectors;

            std::vector< std::vector< CustVector<double> > > separate_vectors = split_to_10<double>(temp_user_vectors);

            double all_sum = 0;
            for (int i = 0; i < 10; i++) {
                double k_sum = 0;
                // Each time query different vector group
                std::vector< CustVector<double> > curr_known = merge_except_for<double>(separate_vectors, i);

                // Begin clustering
                vector<CustVector<double> *> centroids = rand_selection(curr_known, cluster_num);
                //vector<CustVector<double> *> centroids = k_means_pp(user_vectors, cluster_num, metric_type);
                int clustering_iterations = 0;
                bool continue_clustering = true;
                while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
                    lloyds_assignment(curr_known, centroids, metric_type);
                    continue_clustering = k_means(curr_known, centroids, metric_type, min_dist_kmeans);
                    clustering_iterations++;
                }
                std::vector< std::vector<CustVector<double>*> > clusters = separate_clusters_from_input(curr_known,
                        centroids.size());

                for (auto& user : separate_vectors[i]) {
                    // Hide a known value from user
                    //double old_score = hide_one_score(user);

                    std::vector< CustVector<double>* > neighbors = clusters[user.getCluster()];
                    vector<double> similarities = get_P_closest(neighbors, user, P);
                    std::vector<double> pred = get_predicted_user_sim(neighbors, user, similarities);

                    int new_index = user.getUnknownIndexes().at(0);
                    k_sum = k_sum + (old_score - pred[new_index]);
                }
                all_sum = all_sum + (k_sum / separate_vectors.size());

                for (int i = 0; i < centroids.size(); i++)
                    delete centroids[i];
            }
            all_sum = all_sum / 10;

            cout << "Cosine LSH Recommendation MAE: " << all_sum << endl;
        }*/

    }


    /*
     * Clustering Recommendation
     *
     * Part B.
     */

    {
        string metric_type = "euclidean";
        outFile << "Clustering Recommendation" << endl;
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Begin clustering
        vector<CustVector<double> *> centroids = k_means_pp(fake_user_vectors, cluster_num, metric_type);
        int clustering_iterations = 0;
        bool continue_clustering = true;
        while (continue_clustering == true && clustering_iterations < max_algo_iterations) {
            lloyds_assignment(fake_user_vectors, centroids, metric_type);
            continue_clustering = k_means(fake_user_vectors, centroids, metric_type, min_dist_kmeans);
            clustering_iterations++;
        }
        std::vector< std::vector<CustVector<double>*> > clusters = separate_clusters_from_input(fake_user_vectors,
                centroids.size());
        //std::vector<double> sill = silhouette_cluster(fake_user_vectors, centroids, metric_type);

        // Begin calculating optimal recommendations
        for (auto &user : user_vectors) {
            // Find out in what cluster the current user belongs to
            // Assign it to the cluster whose centroid is the closest
            double min_dist = user.euclideanDistance(centroids[0]);
            int min_dist_i = 0;
            for (int centroid_i = 1; centroid_i < centroids.size(); centroid_i++) {
                double curr_dist = user.euclideanDistance(centroids[centroid_i]);
                if (curr_dist < min_dist) {
                    min_dist = curr_dist;
                    min_dist_i = centroid_i;
                }
            }
            std::vector< CustVector<double>* > neighbors = clusters[min_dist_i];
            if (!neighbors.empty()) {
                // Get top 2 recommendations
                // Note that the user vectors have not been normalized yet, only the unknown cryptocurrency values have the
                // mean value. So during this proccess the mean of the vector is subtracted from each rating
                vector<int> recom_crypto_indexes = get_top_N_recom(neighbors, user, 2);
                print_recommendations(outFile, user.getId(), recom_crypto_indexes, query_crypto, 4);
            }
        }

        //std::vector<double> sill = silhouette_cluster(clusters, centroids, metric_type);
        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        outFile << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << endl;
        // If k-means is used then delete centers
        for (int i = 0; i < centroids.size(); i++)
            delete centroids[i];
    }


    /*
     * Program End
     * Free Allocated Memory
     */

    outFile.close();
}


double lsh_rec_10_fold_validation_A(vector< CustVector<double> >& user_vectors, string metric_type, int k, int L,
        int lsh_bucket_div, double euclidean_h_w, int P) {

    std::vector< std::vector< CustVector<double> > > separate_vectors = split_to_10<double>(user_vectors);

    double all_sum = 0;
    for (int i = 0; i < 10; i++) {
        double k_sum = 0;
        // Each time query different vector group
        std::vector< CustVector<double> > curr_known = merge_except_for<double>(separate_vectors, i);
        vector<CustHashtable<double>*> temp_lsh_hashtables = create_LSH_hashtables<double>(curr_known,
                metric_type, k, L, lsh_bucket_div, euclidean_h_w);

        int calc_user_num = 0;
        for (auto& user : separate_vectors[i]) {
            // Hide a known value from user
            double old_score = 0;
            bool calculate = hide_one_score(user, &old_score);

            if (calculate) {
                std::vector<CustVector<double>*> neighbors = get_LSH_filtered_combined_buckets(temp_lsh_hashtables, &user);
                if (!neighbors.empty()) {

                    vector<double> similarities = get_P_closest(neighbors, user, P);
                    std::vector<double> pred = get_predicted_user_sim(neighbors, user, similarities);

                    int new_index = user.getUnknownIndexes().at(0);
                    k_sum = k_sum + fabs(old_score - pred[new_index]);
                    cout << k_sum << " ksum score " << endl;
                    if (isnan(k_sum))
                        int aaaa=0;
                    calc_user_num++;
                }
            }
        }
        if (calc_user_num > 0)
            all_sum = all_sum + (k_sum / calc_user_num);

        for (int i = 0; i < temp_lsh_hashtables.size(); i++)
            delete temp_lsh_hashtables[i];
    }
    all_sum = all_sum / 10;

    return all_sum;
}


/*double lsh_rec_10_fold_validation_B(vector< CustVector<double> >& user_vectors, vector< CustVector<double> >& fake_user_vectors,
        string metric_type, int k, int L, int lsh_bucket_div, double euclidean_h_w, int P) {

    std::vector< std::vector< CustVector<double> > > separate_vectors = split_to_10<double>(user_vectors);

    double all_sum = 0;
    for (int i = 0; i < 10; i++) {
        double k_sum = 0;
        // Each time query different vector group
        std::vector< CustVector<double> > curr_known = merge_except_for<double>(separate_vectors, i);
        vector<CustHashtable<double>*> temp_lsh_hashtables = create_LSH_hashtables<double>(curr_known,
                metric_type, k, L, lsh_bucket_div, euclidean_h_w);

        int calc_user_num = 0;
        for (auto& user : separate_vectors[i]) {
            // Hide a known value from user
            double old_score = 0;
            bool calculate = hide_one_score(user, &old_score);

            if (calculate) {
                std::vector<CustVector<double>*> neighbors = get_LSH_filtered_combined_buckets(temp_lsh_hashtables, &user);
                if (!neighbors.empty()) {

                    vector<double> similarities = get_P_closest(neighbors, user, P);
                    std::vector<double> pred = get_predicted_user_sim(neighbors, user, similarities);

                    int new_index = user.getUnknownIndexes().at(0);
                    k_sum = k_sum + fabs(old_score - pred[new_index]);
                    cout << k_sum << " ksum score " << endl;
                    //if (isnan(k_sum))
                    //    int aaaa=0;
                    calc_user_num++;
                }
            }
        }
        if (calc_user_num > 0)
            all_sum = all_sum + (k_sum / calc_user_num);

        for (int i = 0; i < temp_lsh_hashtables.size(); i++)
            delete temp_lsh_hashtables[i];
    }
    all_sum = all_sum / 10;

    return all_sum;
}*/

double clustering_rec_10_fold_validation();


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


void get_config(string config_file, string* proj_2_input, char* proj_2_csv_delimiter, int* proj_2_cluster_num,
        int* cluster_num, int* k, int* L, int* lsh_bucket_div, double* euclidean_h_w, char* csv_delimiter,
        int* max_algo_iterations, double* min_dist_kmeans, string* lexicon_file, string* query_file) {

    ArgParser* configArgs = new ArgParser( file_to_args(config_file, ' ') );

    if (configArgs->flagExists("number_of_clusters"))
        *cluster_num = stoi( configArgs->getFlagValue("number_of_clusters") );
    else {
        cout << "Please specify the number of clusters to be created" << endl;
        cin >> *cluster_num;
    }
    if (configArgs->flagExists("proj_2_input"))
        *proj_2_input = configArgs->getFlagValue("proj_2_input");
    if (configArgs->flagExists("proj_2_csv_delimiter")) {
        string delim_str = configArgs->getFlagValue("proj_2_csv_delimiter");
        *proj_2_csv_delimiter = delim_str[0];
    }
    if (configArgs->flagExists("proj_2_number_of_clusters"))
        *proj_2_cluster_num = stoi( configArgs->getFlagValue("proj_2_number_of_clusters") );
    if (configArgs->flagExists("number_of_hash_functions"))
        *k = stoi( configArgs->getFlagValue("number_of_hash_functions") );
    if (configArgs->flagExists("number_of_hash_tables"))
        *L = stoi( configArgs->getFlagValue("number_of_hash_tables") );
    if (configArgs->flagExists("lsh_bucket_div"))
        *lsh_bucket_div = stoi( configArgs->getFlagValue("lsh_bucket_div") );
    if (configArgs->flagExists("euclidean_h_w"))
        *euclidean_h_w = stod( configArgs->getFlagValue("euclidean_h_w") );
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
