#ifndef LIB_ARG_PARSER
#define LIB_ARG_PARSER

#include <string>
#include <vector>

/*
 * Argument Parser
 *
 * Argument parser class
 * Can be initialized with either program arguments or just a vector of strings
 *
 * Used in conjuction with split and split_convert functions
 * Useful for checking for flags and returning their values
 */

class ArgParser {
private:
    std::vector<std::string> args;

public:
    ArgParser(int argc, char *argv[]);
    ArgParser(std::vector<std::string> argvec);

    bool flagExists(std::string flag);

    // Get the argument after the input flag, if it exists, else null
    std::string getFlagValue(std::string flag);

    // Get the Nth program argument as a string, if it exists
    std::string getNthArg(int n);
};

#endif //LIB_ARG_PARSER