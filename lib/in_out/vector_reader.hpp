#ifndef LIB_VECTOR_READER_H
#define LIB_VECTOR_READER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "../data_structures/cust_vector.hpp"
#include "../utils.hpp"


/*
 * Vector Reader
 *
 * Class that is used to read vectors from a file, given its name
 * Needs to be altered, for more efficient reading
 *
 * Reads a given number of lines at the start of the file (lines containing metadata to be parsed by ArgParser)
 * Reads vectors from file and creates CustVector objects representing those vectors
 *
 * Templated, so that it can read and create CustVector objects of every type (int, float type dimensions)
 */

template <typename dim_type>
class VectorReader {

private:
    const std::string filename;
    std::vector<std::string> meta_lines;
    std::vector< CustVector<dim_type> > read_vectors;

public:
    VectorReader(std::string name);

    int read(const char delimiter, int strt_line, std::function<dim_type (const std::string&)> conversion_f);

    std::vector< CustVector<dim_type> > getReadVectors();
    std::string getMetaLine(int index);
};


/*
 * Template method definitions
 */


template <typename dim_type>
VectorReader<dim_type>::VectorReader(std::string name) : filename(name) {}


template <typename dim_type>
int VectorReader<dim_type>::read(const char delimiter, int strt_line, std::function<dim_type (const std::string&)> conversion_f) {
    this->meta_lines.empty();
    this->read_vectors.empty();

    std::ifstream input_file;
    input_file.open(filename);
    if ( !input_file.is_open() )
        return -1;

    // First save specified metadata lines (for later parsing)
    std::string line;
    int line_num = 1;
    while (line_num < strt_line && getline(input_file, line)) {
        meta_lines.emplace_back(line);
        line_num++;
    }

    // Get vectors line by line
    while ( getline(input_file, line) ) {
        // Get rid of possible \r characters
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        std::string vector_id = line.substr(0, line.find(delimiter));
        line = line.substr(line.find_first_of(delimiter) + 1);

        CustVector<dim_type> new_vector(vector_id, split_convert<dim_type>(line, delimiter, conversion_f));
        read_vectors.emplace_back(new_vector);
    }

    input_file.close();
    return 1;
}

template <typename dim_type>
std::vector< CustVector<dim_type> > VectorReader<dim_type>::getReadVectors() { return read_vectors; }

template <typename dim_type>
std::string VectorReader<dim_type>::getMetaLine(int index) {
    if (index < meta_lines.size())
        return meta_lines[index];
    else
        return "";
}

#endif //LIB_VECTOR_READER_H
