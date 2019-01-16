#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <vector>

#include "./lib/utils.hpp"

using namespace std;

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

// Example Test case
TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);

}

// Split Test case
TEST_CASE( "Split string into a vector of strings with input delimiter", "[string_split]" ) {
    vector<string> result = split("Test string for catch2", ' ');

    REQUIRE( result.size() == 4);
    REQUIRE( result[0] == "Test");
    REQUIRE( result[1] == "string");
    REQUIRE( result[2] == "for");
    REQUIRE( result[3] == "catch2");

}