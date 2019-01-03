#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "arg_parser.h"

using namespace std;

ArgParser::ArgParser(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++)
        args.emplace_back( string(argv[i]) );
}


ArgParser::ArgParser(vector<string> argvec) : args(argvec) {};


bool ArgParser::flagExists(string flag) {
    return find(args.begin(), args.end(), flag) != args.end();
}


string ArgParser::getFlagValue(string flag) {
    // Should I set it to auto?
    vector<string>::iterator it = find(args.begin(), args.end(), flag);
    if (it != args.end() && it + 1 != args.end())
        return *(it + 1);
    else
        return NULL;
}


string ArgParser::getNthArg(int n) {
    if (n >= 0 && n < args.size())
        return args[n];
    else
        return NULL;
}



