#ifndef LIB_TWEET_H
#define LIB_TWEET_H

#include <vector>
#include <string>

class Tweet {
private:
    std::string id;
    int crypto_index;
    double sentiment_score;

public:
    Tweet();
    ~Tweet();

    void

    unsigned long getSize();
};


#endif //LIB_TWEET_H
