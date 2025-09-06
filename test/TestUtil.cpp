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
    REQUIRE(htask::util::str_to_scaled_num("111209.90000000", 2) == 11120990);
}

TEST_CASE("scale_down_to_str", "[util]") {
    REQUIRE(htask::util::scale_down_to_str(0, 4) == "0");
    REQUIRE(htask::util::scale_down_to_str(1, 4) == "0.0001");
    REQUIRE(htask::util::scale_down_to_str(12, 4) == "0.0012");
    REQUIRE(htask::util::scale_down_to_str(123, 4) == "0.0123");
    REQUIRE(htask::util::scale_down_to_str(1234, 4) == "0.1234");
    REQUIRE(htask::util::scale_down_to_str(12345, 4) == "1.2345");
    REQUIRE(htask::util::scale_down_to_str(123456, 4) == "12.3456");
}
