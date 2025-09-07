#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch2/catch_all.hpp>
#include "util.h"

using namespace htask::util;
using std::cout, std::endl;

TEST_CASE("normalize num", "[util]") {
    REQUIRE(normalizeNum("123.400") == "123.4");
    REQUIRE(normalizeNum("123") == "123");
    REQUIRE(normalizeNum("123.00") == "123");
}

TEST_CASE("str_to_scaled_num", "[util]") {
    REQUIRE(str_to_scaled_num("123.400", 3) == 123400);
    REQUIRE(str_to_scaled_num("123.40", 3) == 123400);
    REQUIRE(str_to_scaled_num("123.04", 3) == 123040);
    REQUIRE(str_to_scaled_num("123.4", 3) == 123400);
    REQUIRE(str_to_scaled_num("123", 3) == 123000);
    REQUIRE(str_to_scaled_num("111209.90000000", 2) == 11120990);
    // no overflow
    REQUIRE(str_to_scaled_num("10000000", 8) == 1'000'000'000'000'000);
    // max number we ever use - no overflow
    REQUIRE(str_to_scaled_num("50000000", 8) == 5'000'000'000'000'000);
}

TEST_CASE("scale_down_to_str", "[util]") {
    REQUIRE(scale_down_to_str(0, 4) == "0");
    REQUIRE(scale_down_to_str(1, 4) == "0.0001");
    REQUIRE(scale_down_to_str(12, 4) == "0.0012");
    REQUIRE(scale_down_to_str(123, 4) == "0.0123");
    REQUIRE(scale_down_to_str(1234, 4) == "0.1234");
    REQUIRE(scale_down_to_str(12345, 4) == "1.2345");
    REQUIRE(scale_down_to_str(123456, 4) == "12.3456");
}
