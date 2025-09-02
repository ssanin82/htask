#include <utility>
#include <format>
#include <iostream>
#include <iomanip>

#include "OrderBook.h"

using std::cout, std::endl;

constexpr int PRINT_LV = 10;

namespace htask {
namespace util {

void OrderBook::updateLevel(
    bool isBid, const std::string& price, const std::string& size
) {
    if (isBid) {
        if (size == "0" && bids.contains(price)) bids.erase(price);
        else bids[price] = size;
    } else {
        if (size == "0" && asks.contains(price)) asks.erase(price);
        else asks[price] = size;
    }
}

std::pair<std::string, std::string> OrderBook::getBestBid() {
    if (bids.empty()) return std::make_pair("", "");
    else return *bids.begin();
}

void OrderBook::print() {
    cout << "BIDS (size=" << bids.size() << "):" << endl;
    auto it = bids.begin();
    int count = PRINT_LV;
    while (it != bids.end() && count) {
        cout << std::format("price: {}, size: {}", it->first, it->second) << endl;
        ++it;
        --count;
    }
    double liquidity = 0.;
    for (auto& [k, v]: bids) liquidity += std::stod(k) * std::stod(v);
    cout << "BID VOLUME: " << std::fixed << std::setprecision(2)
        << liquidity << "(" << (liquidity / 1'000'000) << " mln)" << endl;
    cout << endl;

    cout << "ASKS (size=" << asks.size() << "):" << endl;
    it = asks.begin();
    count = PRINT_LV;
    while (it != asks.end() && count) {
        cout << std::format("price: {}, size: {}", it->first, it->second) << endl;
        ++it;
        --count;
    }
    liquidity = 0.;
    for (auto& [k, v]: asks) liquidity += std::stod(k) * std::stod(v);
    cout << "ASK VOLUME: " << std::fixed << std::setprecision(2)
        << liquidity << "(" << (liquidity / 1'000'000) << " mln)" << endl;
    cout << endl;
}

}
}
