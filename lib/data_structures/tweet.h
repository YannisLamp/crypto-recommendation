#ifndef LIB_TWEET_H
#define LIB_TWEET_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>

/*
 * Custom Hashtable
 *
 * Hashtable class used for LSH to store vectors in buckets
 * Therefore, contains a given number of buckets and a hash generator to determine
 * in which bucket an input vector should be placed
 *
 * Accepts any HashGenerator object as its hash generator, so that it can be used for
 * any metric we want to use
 *
 * Templated, so that it can store any type of vector (int, float type dimensions)
 *
 *
 *
 * NO NEED TO KEEP TWEET'S ACTUAL WORDS
 */


class Tweet {
private:
    std::string id;
    std::string user_id;
    std::set<int> crypto_indexes;
    double sentiment_score;

public:
    // EXPAPAPPAPAPAPP CALCULATE
    Tweet(std::vector<std::string>& tweet_words, std::unordered_map<std::string, float>& lexicon,
            std::vector< std::vector<std::string> >& query_crypto);

    // Getters for tweet stats
    std::string getId();
    std::string getUserId();
    std::vector<int> getCryptoIndexes();
    double getSentimentScore();
};


#endif //LIB_TWEET_H
