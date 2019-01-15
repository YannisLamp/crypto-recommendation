#ifndef LIB_TWEET_H
#define LIB_TWEET_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>

/*
 * Tweet
 *
 * Tweet class used for cryptocurrency recommendation, it stores the tweet's id, the id of the iser that posted it,
 * the indexes of the cryptocurrencies that are mentioned in it (from the input cryptocurrency query file) and
 * its overall sentiment score
 *
 * Its constructor accepts the actual tweet (words), a lexicon for scoring the tweet and the query words,
 * in this case the different words representing different cryptocurrencies, then the tweet's sentiment
 * score is calculated
 *
 * There is no need to save the actual tweet words
 *
 */


class Tweet {
private:
    std::string id;
    std::string user_id;
    std::set<int> crypto_indexes;
    double sentiment_score;

public:
    // Store essential tweet information and calculate overall sentiment score
    Tweet(std::vector<std::string>& tweet_words, std::unordered_map<std::string, float>& lexicon,
            std::vector< std::vector<std::string> >& query_crypto);

    // Getters for tweet stats
    std::string getId();
    std::string getUserId();
    std::vector<int> getCryptoIndexes();
    double getSentimentScore();
};


#endif //LIB_TWEET_H
