#include <iostream>
#include <thread>
#include <chrono>

// #include "md_binance.h"
#include "md_okx.h"
// #include "md_gateio.h"

int main() {
    // std::jthread t(htask::md_gateio::work);
    std::jthread t(htask::md_okx::work);
    while (true) std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
