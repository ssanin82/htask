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

// basics for bid

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

TEST_CASE_METHOD(ObTestFixture, "Order book bbid delete all", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "123.45", "10");
    ob.updateLevel(MktData::Binance, true, "123.45", "0");
    auto bbid = ob.getBestBid();
    REQUIRE(!bbid.first);
    REQUIRE(!bbid.second);
}

// basics for asks: usually, they are in the separate conditional branch,
// so need separate tests for a better coverage

TEST_CASE_METHOD(ObTestFixture, "Order book bask", "[fixture]") {
    ob.updateLevel(MktData::GateIo, false, "123.45", "10");
    ob.updateLevel(MktData::Okx, false, "123.45", "11");
    auto bask = ob.getBestAsk();
    REQUIRE(eq(123.45, scale_down(bask.first, PRICE_SCALE)));
    REQUIRE(eq(21, scale_down(bask.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bask replace", "[fixture]") {
    ob.updateLevel(MktData::Binance, false, "123.45", "10");
    ob.updateLevel(MktData::Okx, false, "123.45", "11");
    ob.updateLevel(MktData::Okx, false, "123.45", "12");
    auto bask = ob.getBestAsk();
    REQUIRE(eq(123.45, scale_down(bask.first, PRICE_SCALE)));
    REQUIRE(eq(22, scale_down(bask.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bask delete", "[fixture]") {
    ob.updateLevel(MktData::Binance, false, "123.45", "10");
    ob.updateLevel(MktData::Okx, false, "123.45", "11");
    ob.updateLevel(MktData::Okx, false, "123.45", "0");
    auto bask = ob.getBestAsk();
    REQUIRE(eq(123.45, scale_down(bask.first, PRICE_SCALE)));
    REQUIRE(eq(10, scale_down(bask.second, SIZE_SCALE)));
}

TEST_CASE_METHOD(ObTestFixture, "Order book bask delete all", "[fixture]") {
    ob.updateLevel(MktData::Binance, false, "123.45", "10");
    ob.updateLevel(MktData::Binance, false, "123.45", "0");
    auto bask = ob.getBestAsk();
    REQUIRE(!bask.first);
    REQUIRE(!bask.second);
}

// combined

TEST_CASE_METHOD(ObTestFixture, "Order book bask/bask combined", "[fixture]") {
    // asks
    ob.updateLevel(MktData::GateIo, false, "123.46", "10");
    ob.updateLevel(MktData::Binance, false, "123.45", "11");
    ob.updateLevel(MktData::Okx, false, "123.45", "11");
    ob.updateLevel(MktData::Binance, false, "123.44", "11");
    // bids
    ob.updateLevel(MktData::Binance, true, "123.43", "10");
    ob.updateLevel(MktData::Okx, true, "123.42", "11");
    ob.updateLevel(MktData::GateIo, true, "123.41", "11");
    ob.updateLevel(MktData::Binance, true, "123.43", "0"); // deletion
    //
    auto bask = ob.getBestAsk();
    REQUIRE(eq(123.44, scale_down(bask.first, PRICE_SCALE)));
    REQUIRE(eq(11, scale_down(bask.second, SIZE_SCALE)));
    auto bbid = ob.getBestBid();
    REQUIRE(eq(123.42, scale_down(bbid.first, PRICE_SCALE)));
    REQUIRE(eq(11, scale_down(bbid.second, SIZE_SCALE)));
}

// others

TEST_CASE_METHOD(ObTestFixture, "Volume pricer", "[fixture]") {
    ob.updateLevel(MktData::Binance, true, "1", "10");
    ob.updateLevel(MktData::Binance, true, "2", "8");
    ob.updateLevel(MktData::Binance, true, "3", "6");
    REQUIRE(eq(3., scale_down(ob.getVolumePrice(true, 10),PRICE_SCALE)));
    REQUIRE(eq(3., scale_down(ob.getVolumePrice(true, 18),PRICE_SCALE)));
    REQUIRE(eq(2.85, scale_down(ob.getVolumePrice(true, 20),PRICE_SCALE)));
    REQUIRE(eq(1.83, scale_down(ob.getVolumePrice(true, 44),PRICE_SCALE)));
    REQUIRE(eq(0., scale_down(ob.getVolumePrice(true, 45),PRICE_SCALE)));
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

TEST_CASE_METHOD(ObTestFixture, "getVolumePriceMln", "[fixture]") {
    ob.updateLevel(MktData::GateIo, false, "1", "3000000");
    ob.updateLevel(MktData::Okx, false, "2", "2000000");
    ob.updateLevel(MktData::GateIo, false, "3", "1000000");
    REQUIRE(eq(1., scale_down(ob.getVolumePriceMln(false, 2), PRICE_SCALE)));
    REQUIRE(eq(1.14, scale_down(ob.getVolumePriceMln(false, 4), PRICE_SCALE)));
    REQUIRE(eq(1.66, scale_down(ob.getVolumePriceMln(false, 10), PRICE_SCALE)));
    REQUIRE(eq(0., scale_down(ob.getVolumePriceMln(false, 11), PRICE_SCALE)));
}
