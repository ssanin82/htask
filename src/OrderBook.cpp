#include <utility>
#include <format>
#include <iostream>
#include <iomanip>

#include "OrderBook.h"

using std::cout, std::endl, std::string;

constexpr int PRINT_LV = 10;

namespace htask {
namespace util {

string normalize(const string& s) {
    string s1;
    if(s.find('.') != string::npos) {
        s1 = s.substr(0, s.find_last_not_of('0') + 1);
        if(s1.find('.') == s1.size() - 1) s1 = s1.substr(0, s1.size()-1);
    }
    return s1;
}

void OrderBook::updateLevel(
    bool isBid, const string& price, const string& size
) {
    string key = normalize(price);
    int updCount = 0;
    int rmCount = 0;
    if (isBid) {
        if (size == "0" && bids.contains(key)) {
            bids.erase(key);
            ++rmCount;
        }
        else {
            bids[key] = size;
            ++updCount;
        }
    } else {
        if (size == "0" && asks.contains(key)) {
            asks.erase(key);
            ++rmCount;
        }
        else {
            asks[key] = size;
            ++updCount;
        }
    }
}

std::pair<string, string> OrderBook::getBestBid() {
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

void OrderBook::clear() {
    bids.clear();
    asks.clear();
}

}
}
