#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch2/catch_all.hpp>
#include "OrderBook.h"
#include "util.h"

using std::cout, std::endl;

class ObFixture {
public:
    ObFixture() { /* Setup code */ }
    ~ObFixture() { /* Teardown code */ }

    htask::util::OrderBook ob;
};

TEST_CASE_METHOD(ObFixture, "Order book bbid", "[fixture]") {
    ob.updateLevel(htask::util::MktData::Binance, true, "123.456", "10");
    ob.updateLevel(htask::util::MktData::Okx, true, "123.456", "11");
    auto bbid = ob.getBestBid();
    REQUIRE("123.456" == bbid.first);
    REQUIRE("21" == bbid.second);
}

TEST_CASE_METHOD(ObFixture, "Order book bbid replace", "[fixture]") {
    ob.updateLevel(htask::util::MktData::Binance, true, "123.456", "10");
    ob.updateLevel(htask::util::MktData::Okx, true, "123.456", "11");
    ob.updateLevel(htask::util::MktData::Okx, true, "123.456", "12");
    auto bbid = ob.getBestBid();
    REQUIRE("123.456" == bbid.first);
    REQUIRE("22" == bbid.second);
}

TEST_CASE_METHOD(ObFixture, "Order book bbid delete", "[fixture]") {
    ob.updateLevel(htask::util::MktData::Binance, true, "123.456", "10");
    ob.updateLevel(htask::util::MktData::Okx, true, "123.456", "11");
    ob.updateLevel(htask::util::MktData::Okx, true, "123.456", "0");
    auto bbid = ob.getBestBid();
    REQUIRE("123.456" == bbid.first);
    REQUIRE("10" == bbid.second);
}

TEST_CASE_METHOD(ObFixture, "Order book bask delete all", "[fixture]") {
    ob.updateLevel(htask::util::MktData::Binance, true, "123.456", "10");
    ob.updateLevel(htask::util::MktData::Binance, true, "123.456", "0");
    auto bbid = ob.getBestBid();
    REQUIRE("" == bbid.first);
    REQUIRE("" == bbid.second);
}

TEST_CASE_METHOD(ObFixture, "Volume pricer1", "[fixture]") {
    ob.updateLevel(htask::util::MktData::Binance, true, "1", "10");
    ob.updateLevel(htask::util::MktData::Binance, true, "2", "8");
    ob.updateLevel(htask::util::MktData::Binance, true, "3", "6");
    REQUIRE(htask::util::eq(3., ob.getVolumePrice(true, 10.)));
    REQUIRE(htask::util::eq(3., ob.getVolumePrice(true, 18.)));
    REQUIRE(htask::util::eq(20. / 7, ob.getVolumePrice(true, 20.)));
    REQUIRE(htask::util::eq(1.833333333, ob.getVolumePrice(true, 44.)));
    REQUIRE(htask::util::eq(-1., ob.getVolumePrice(true, 45.)));
}
