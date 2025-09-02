#include <utility>
#include <format>
#include <iostream>
#include <iomanip>
#include <vector>

#include "OrderBook.h"
#include "util.h"

using std::cout, std::endl, std::string, std::pair, std::make_pair;

constexpr int PRINT_LV = 10;

namespace htask {
namespace util {

void OrderBook::updateLevel(
    MktData md, bool isBid, const string& price, const string& size
) {
    string key = normalizeNum(price);
    if (isBid) {
        if (size == "0" && bids.contains(key)) {
            bids[key].erase(md);
            if (bids[key].empty()) bids.erase(key);
        } else bids[key][md] = size;
    } else {
        if (size == "0" && asks.contains(key)) {
            asks[key].erase(md);
            if (asks[key].empty()) asks.erase(key);
        } else asks[key][md] = size;
    }
}

string OrderBook::getSize(bool isBid, const string& price) {
    auto& best = isBid ? bids.begin()->second : asks.begin()->second;
    double sz = 0.;
    for (const auto& el: best) sz += stod(el.second);
    std::stringstream ss;
    ss << std::setprecision(3) << sz;
    return ss.str();
}

std::pair<std::string, std::string> OrderBook::getBest(bool isBid) {
    if (isBid && bids.empty() || !isBid && asks.empty()) {
        return std::make_pair("", "");
    }
    auto& price = isBid ? bids.begin()->first : asks.begin()->first;
    return make_pair(price, getSize(isBid, price));
}

std::pair<string, string> OrderBook::getBestBid() {
    return getBest(true);
}

std::pair<string, string> OrderBook::getBestAsk() {
    return getBest(false);
}

double OrderBook::getVolumePrice(bool isBid, double target) {
    auto lvIt = isBid ? bids.begin() : asks.begin();
    auto lvEnd = isBid ? bids.end() : asks.end();
    double lq = 0.;
    std::vector<pair<double, double>> acc;
    while (lvIt != lvEnd) {
        double price = stod(lvIt->first);
        double sz = 0.;
        for (auto& [_, v]: lvIt->second) sz += stod(v);
        double levelNotional = price * sz;
        if (gt(lq + levelNotional, target)) {
            acc.push_back(make_pair(price, (target - lq) / price));
            lq = target;
            break;
        } else {
            acc.push_back(make_pair(price, sz));
            lq += levelNotional;
        }
        ++lvIt;
    }
    if (gt(target, lq)) return -1.; // insufficient volume
    else {
        double notional = 0.;
        double qty = 0.;
        for (auto& [p, q]: acc) {
            notional += p * q;
            qty += q;
        }
        return notional / qty;
    }
    return 0.;
}

double OrderBook::getVolumePriceMln(bool isBid, int x) {
    if (!x) return -1;
    return getVolumePrice(isBid, x * 1'000'000.);
}

void OrderBook::print() {
    cout << "BIDS (size=" << bids.size() << "):" << endl;
    auto it = bids.begin();
    int count = PRINT_LV;
    while (it != bids.end() && count) {
        string price = it->first;
        string sz = getSize(true, price);
        cout << std::format("price: {}, size: {}", price, sz) << endl;
        ++it;
        --count;
    }
    double liquidity = 0.;
    for (auto& [k, v]: bids) liquidity += std::stod(k) * stod(getSize(true, k));
    cout << "BID VOLUME: " << std::fixed << std::setprecision(2)
        << liquidity << "(" << (liquidity / 1'000'000) << " mln)" << endl;
    cout << endl;

    cout << "ASKS (size=" << asks.size() << "):" << endl;
    it = asks.begin();
    count = PRINT_LV;
    while (it != asks.end() && count) {
        string price = it->first;
        string sz = getSize(false, price);
        cout << std::format("price: {}, size: {}", price, sz) << endl;
        ++it;
        --count;
    }
    liquidity = 0.;
    for (auto& [k, v]: asks) liquidity += std::stod(k) * stod(getSize(false, k));
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
