#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch2/catch_all.hpp>
#include "util.h"

using std::cout, std::endl;

TEST_CASE("normalize num", "[util]") {
    REQUIRE(htask::util::normalizeNum("123.400") == "123.4");
    REQUIRE(htask::util::normalizeNum("123") == "123");
    REQUIRE(htask::util::normalizeNum("123.00") == "123");
}

TEST_CASE("str_to_scaled_num", "[util]") {
    REQUIRE(htask::util::str_to_scaled_num("123.400", 3) == 123400);
    REQUIRE(htask::util::str_to_scaled_num("123.40", 3) == 123400);
    REQUIRE(htask::util::str_to_scaled_num("123.04", 3) == 123040);
    REQUIRE(htask::util::str_to_scaled_num("123.4", 3) == 123400);
    REQUIRE(htask::util::str_to_scaled_num("123", 3) == 123000);
}
