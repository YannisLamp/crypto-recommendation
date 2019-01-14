#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

#include "tweet.h"

using namespace std;

Tweet::Tweet(vector<string>& tweet_words, unordered_map<string, float>& lexicon,
        vector< vector<string> >& query_crypto) : sentiment_score(0) {
    // For each word the tweet contains
    user_id = tweet_words[0];
    id = tweet_words[1];

    double totalscore = 0;
    for (auto i = 2; i < tweet_words.size(); i++) {
        // Check if it exists in the input lexicon
        if (lexicon.count(tweet_words[i]) > 0) {
            // If a word is found then add its sentiment score to the overall score of the tweet
            totalscore = totalscore + lexicon[tweet_words[i]];
        }
        // Else check if it represents a cryptocurrency, going through all variations of each one
        else {
            // For each word of the tweet
            int coin_index = 0;
            for (auto& currency : query_crypto) {
                for (auto& variation : currency) {
                    if (tweet_words[i] == variation)
                        crypto_indexes.insert(coin_index);
                }

                coin_index++;
            }
        }
    }

    // Now calculate overall tweet sentiment score with given formula
    int alpha = 15;
    sentiment_score = totalscore / sqrt( totalscore*totalscore + alpha );
}


string Tweet::getId() { return id; }


string Tweet::getUserId() { return user_id; }


vector<int> Tweet::getCryptoIndexes() {
    std::vector<int> crypto_indexes_vector(crypto_indexes.begin(), crypto_indexes.end());
    return crypto_indexes_vector;
}


double Tweet::getSentimentScore() { return sentiment_score; }