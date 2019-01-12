#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "utils.hpp"

using namespace std;

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


vector<int> get_num_hamming_dist_from(int num, int dist, int min_bit, int bits) {
    int mask = 1;
    // Shift the bit of the mask left a given number of times
    // To achieve the right combinations through recursions
    for (int i = 0; i < min_bit; i++)
        mask = mask << 1;

    vector<int> res_numbers;
    for (int i = min_bit; i < bits; i++) {
        int temp = num ^ mask;

        // If the current distance is greater than one (so this is not the last recursion)
        // Call this function with one hamming distance less and one more bit to the left for the mask
        if (dist > 1) {
            vector<int> rec_res = get_num_hamming_dist_from(temp, dist - 1, i + 1, bits);

            // Append all the solutions for this distance
            res_numbers.reserve( res_numbers.size() + rec_res.size() );
            res_numbers.insert( res_numbers.end(), rec_res.begin(), rec_res.end());
        }
        else if (dist == 1) {
            res_numbers.emplace_back(temp);
        }

        mask = mask << 1;
    }

    return res_numbers;
}


vector<string> file_to_args(string filename, char delimiter) {
    vector<string> args;

    ifstream input_file;
    input_file.open(filename);
    if ( !input_file.is_open() )
        return args;

    string line;
    while (getline(input_file, line)) {
        vector<string> line_args = split(line, delimiter);
        args.insert(args.end(), line_args.begin(), line_args.end());
    }

    input_file.close();
    return args;
}


vector< vector<string> > file_to_str_vectors(string filename, char delimiter) {
    vector< vector<string> > vecs;

    ifstream input_file;
    input_file.open(filename);
    if ( !input_file.is_open() )
        return vecs;

    string line;
    while (getline(input_file, line)) {
        vector<string> current_vector = split(line, delimiter);
        vecs.emplace_back(current_vector);
    }

    input_file.close();
    return vecs;
}


std::unordered_map<string, float> file_to_lexicon(string filename, char delimiter) {
    std::unordered_map<string, float> lexicon;

    ifstream input_file;
    input_file.open(filename);
    if ( !input_file.is_open() )
        return lexicon;

    string line;
    while (getline(input_file, line)) {
        vector<string> current_word = split(line, delimiter);
        float score = stof(current_word[1]);
        lexicon.emplace(current_word[0], score);
    }

    input_file.close();
    return lexicon;
}
