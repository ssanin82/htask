#include <iostream>
#include <thread>
#include <chrono>

#include "OrderBook.h"
#include "md/binance.h"
#include "md/okx.h"
#include "md/gateio.h"

int main() {
    htask::util::OrderBook ob;

    std::jthread tBinance(htask::md_binance::work, std::ref(ob));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // XXX binance needs some time to synchronize initial order book
    // snapshit with the buffered updates

    std::jthread tGateIo(htask::md_gateio::work, std::ref(ob));
    std::jthread tOkx(htask::md_okx::work, std::ref(ob));
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        ob.print();
        // ob.printExtended();
    }
    return 0;
}
