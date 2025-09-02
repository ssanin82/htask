#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include "OrderBook.h"

class ObFixture {
public:
    ObFixture() { /* Setup code */ }
    ~ObFixture() { /* Teardown code */ }

    htask::util::OrderBook ob;
};

TEST_CASE_METHOD(ObFixture, "Order book tests" "[fixture]") {
    ob.updateLevel(true, "123.456", "10");
    ob.getBestBid();
}
