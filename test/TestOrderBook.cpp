#define CATCH_CONFIG_MAIN

#include <iostream>

#include <catch2/catch_all.hpp>
#include "OrderBook.h"
#include "util.h"

using std::cout, std::endl;
using namespace htask::util;

class ObTestFixture {
public:
    ObTestFixture() { /* Setup code */ }
    ~ObTestFixture() { /* Teardown code */ }

    OrderBook ob;
};

TEST_CASE_METHOD(ObTestFixture, "Order book bbid", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "123.45", "10");
    ob.updateLevel(MktData::Okx, true, "123.45", "11");
    auto bbid = ob.getBestBid();
    REQUIRE(eq(123.45, scale_down(bbid.first, PRICE_SCALE)));
    REQUIRE(eq(21, scale_down(bbid.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bbid replace", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "123.45", "10");
    ob.updateLevel(MktData::Okx, true, "123.45", "11");
    ob.updateLevel(MktData::Okx, true, "123.45", "12");
    auto bbid = ob.getBestBid();
    REQUIRE(eq(123.45, scale_down(bbid.first, PRICE_SCALE)));
    REQUIRE(eq(22, scale_down(bbid.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bbid delete", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "123.45", "10");
    ob.updateLevel(MktData::Okx, true, "123.45", "11");
    ob.updateLevel(MktData::Okx, true, "123.45", "0");
    auto bbid = ob.getBestBid();
    REQUIRE(eq(123.45, scale_down(bbid.first, PRICE_SCALE)));
    REQUIRE(eq(10, scale_down(bbid.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bask delete all", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "123.45", "10");
    ob.updateLevel(MktData::Binance, true, "123.45", "0");
    auto bbid = ob.getBestBid();
    REQUIRE(!bbid.first);
    REQUIRE(!bbid.second);
}

TEST_CASE_METHOD(ObTestFixture, "Volume pricer", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "1", "10");
    ob.updateLevel(MktData::Binance, true, "2", "8");
    ob.updateLevel(MktData::Binance, true, "3", "6");
    REQUIRE(eq(
        3.,
        scale_down(ob.getVolumePrice(true, 10),PRICE_SCALE)
    ));
    REQUIRE(eq(
        3.,
        scale_down(ob.getVolumePrice(true, 18),PRICE_SCALE)
    ));
    REQUIRE(eq(
        2.85,
        scale_down(ob.getVolumePrice(true, 20),PRICE_SCALE)
    ));
    REQUIRE(eq(
        1.83,
        scale_down(ob.getVolumePrice(true, 44),PRICE_SCALE)
    ));
    REQUIRE(eq(
        0.,
        scale_down(ob.getVolumePrice(true, 45),PRICE_SCALE)
    ));
}

TEST_CASE_METHOD(ObTestFixture, "Mid price", "[fixture]") {
    ob.updateLevel(MktData::GateIo, false, "3.4", "10");
    ob.updateLevel(MktData::Okx, false, "3.3", "10");
    ob.updateLevel(MktData::GateIo, false, "3.2", "10");
    ob.updateLevel(MktData::Binance, true, "3", "10");
    ob.updateLevel(MktData::Okx, true, "2.9", "10");
    ob.updateLevel(MktData::Binance, true, "2.8", "10");
    REQUIRE(eq(3.1, scale_down(ob.getMidPrice(), PRICE_SCALE)));
}
