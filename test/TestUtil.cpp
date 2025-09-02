#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch2/catch_all.hpp>
#include "util.h"

using std::cout, std::endl;

TEST_CASE("normalize num 1", "[util]") {
    REQUIRE(htask::util::normalizeNum("123.400") == "123.4");
    REQUIRE(htask::util::normalizeNum("123") == "123");
    REQUIRE(htask::util::normalizeNum("123.00") == "123");
}
