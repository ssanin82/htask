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
    std::lock_guard<std::recursive_mutex> lg(lock);
    PRICE_T key = str_to_scaled_num(price, PRICE_SCALE);
    SIZE_T sz = str_to_scaled_num(size, SIZE_SCALE);
    if (isBid) {
        if (!sz) {
            if (bids.contains(key)) {
                bids.at(key).erase(md);
                if (bids.at(key).empty()) bids.erase(key);
            }
            if (xchBids.contains(md) && xchBids[md].contains(key)) {
                xchBids[md].erase(key);
            }
        } else {
            bids[key][md] = sz;
            xchBids[md][key] = sz;
        }
    } else {
        if (!sz) {
            if (asks.contains(key)) {
                asks.at(key).erase(md);
                if (asks.at(key).empty()) asks.erase(key);
            }
            if (xchAsks.contains(md) && xchAsks[md].contains(key)) {
                xchAsks[md].erase(key);
            }
        } else  {
            asks[key][md] = sz;
            xchAsks[md][key] = sz;
        }
    }
}

SIZE_T OrderBook::getSize(bool isBid, PRICE_T price) {
    std::lock_guard<std::recursive_mutex> lg(lock);
    if (isBid) {
        auto it = bids.find(price);
        if (it != bids.end()) {
            SIZE_T sz = NO_SIZE;
            for (const auto& el: it->second) sz += el.second;
            return sz;
        }
    } else {
        auto it = asks.find(price);
        if (it != asks.end()) {
            SIZE_T sz = NO_SIZE;
            for (const auto& el: it->second) sz += el.second;
            return sz;
        }
    }
    return 0LL;
}

std::pair<PRICE_T, SIZE_T> OrderBook::getBest(bool isBid) {
    std::lock_guard<std::recursive_mutex> lg(lock);
    if (isBid && bids.empty() || !isBid && asks.empty()) {
        return std::make_pair(0, NO_SIZE);
    }
    PRICE_T price = isBid ? bids.begin()->first : asks.begin()->first;
    return make_pair(price, getSize(isBid, price));
}

std::pair<PRICE_T, SIZE_T> OrderBook::getBestBid() {
    return getBest(true);
}

std::pair<PRICE_T, SIZE_T> OrderBook::getBestAsk() {
    return getBest(false);
}

PRICE_T OrderBook::getVolumePrice(bool isBid, SIZE_T target_usdt) {
    std::lock_guard<std::recursive_mutex> lg(lock);
    target_usdt = scale_up(target_usdt, SIZE_SCALE); // scale the target notional
    target_usdt = scale_up(target_usdt, PRICE_SCALE);
    auto lvIt = isBid ? bids.begin() : asks.begin();
    auto lvEnd = isBid ? bids.end() : asks.end();
    SIZE_T notionalSoFar = NO_SIZE;
    std::vector<pair<PRICE_T, SIZE_T>> acc;
    while (lvIt != lvEnd) {
        PRICE_T price = lvIt->first;
        SIZE_T sz = NO_SIZE;
        for (auto& [p, v]: lvIt->second) {
            sz += v;
        }
        SIZE_T levelNotional = price * sz;
        if (gt(notionalSoFar + levelNotional, target_usdt)) {
            // XXX division may create imprecision, but that's not critical here
            acc.push_back(make_pair(
                price, (target_usdt - notionalSoFar) / price)
            );
            notionalSoFar = target_usdt;
            break;
        } else {
            acc.push_back(make_pair(price, sz));
            notionalSoFar += levelNotional;
        }
        ++lvIt;
    }
    if (gt(target_usdt, notionalSoFar)) return 0; // insufficient volume
    else {
        double notional = 0.;
        double qty = 0.;
        for (auto& [p, q]: acc) {
            notional += p * q;
            qty += q;
        }
        return notional / qty;
    }
    return 0;
}

PRICE_T OrderBook::getVolumePriceMln(bool isBid, int x) {
    if (!x) return 0;
    return getVolumePrice(isBid, x * 1'000'000);
}

void OrderBook::print() {
    std::lock_guard<std::recursive_mutex> lg(lock);
    auto bbidIt = bids.begin();
    cout << "BIDS (size=" << bids.size() << "):" << endl;
    auto it = bids.begin();
    int count = PRINT_LV;
    while (it != bids.end() && count) {
        PRICE_T price = it->first;
        SIZE_T sz = getSize(true, price);
        SIZE_T binanceSz = it->second.contains(MktData::Binance) ?
            it->second[MktData::Binance] : 0;
        SIZE_T okxSz = it->second.contains(MktData::Okx) ?
            it->second[MktData::Okx] : 0;
        SIZE_T gateioSz = it->second.contains(MktData::GateIo) ?
            it->second[MktData::GateIo] : 0;
        cout << std::fixed
            << std::setprecision(PRICE_SCALE)
            << "price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE)
            << " -- "
            << "binance: " << scale_down(binanceSz, SIZE_SCALE)
            << ", okx: " << scale_down(okxSz, SIZE_SCALE)
            << ", gateio: " << scale_down(gateioSz, SIZE_SCALE) << endl; 
        ++it;
        --count;
    }
    SIZE_T liquidity = NO_SIZE;
    for (auto& [k, v]: bids) liquidity += k * getSize(true, k);
    cout << "BID VOLUME: (" << std::fixed << std::setprecision(2)
        << scale_down(liquidity, PRICE_SCALE + SIZE_SCALE) / 1'000'000
        << " mln)" << endl;
    cout << endl;

    cout << "ASKS (size=" << asks.size() << "):" << endl;
    it = asks.begin();
    count = PRINT_LV;
    while (it != asks.end() && count) {
        PRICE_T price = it->first;
        SIZE_T sz = getSize(false, price);
        SIZE_T binanceSz = it->second.contains(MktData::Binance) ?
            it->second[MktData::Binance] : 0;
        SIZE_T okxSz = it->second.contains(MktData::Okx) ?
            it->second[MktData::Okx] : 0;
        SIZE_T gateioSz = it->second.contains(MktData::GateIo) ?
            it->second[MktData::GateIo] : 0;
        cout << std::fixed
            << std::setprecision(PRICE_SCALE)
            << "price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE)
            << " -- "
            << "binance: " << scale_down(binanceSz, SIZE_SCALE)
            << ", okx: " << scale_down(okxSz, SIZE_SCALE)
            << ", gateio: " << scale_down(gateioSz, SIZE_SCALE) << endl;
        ++it;
        --count;
    }
    liquidity = NO_SIZE;
    for (auto& [k, v]: asks) liquidity += k * getSize(false, k);
    cout << "ASK VOLUME: (" << std::fixed << std::setprecision(2)
        << scale_down(liquidity, PRICE_SCALE + SIZE_SCALE) / 1'000'000
        << " mln)" << endl;
    cout << endl;
}

void OrderBook::printXchBook(MktData md) {
    if (!xchBids.contains(md) || !xchAsks.contains(md)) {
        cout << "no data available" << endl;
        return;
    }
    auto itX = xchBids[md].begin();
    int lv = 0;
    while (itX != xchBids[md].end() && lv < PRINT_LV) {
        ++lv;
        PRICE_T price = itX->first;
        SIZE_T sz = itX->second;;
        cout << "bid " << std::setw(3) << std::setfill('0') << lv
            << std::fixed << std::setprecision(PRICE_SCALE)
            << " price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE) << endl; 
        ++itX;
    }
    cout << "#" << endl;
    itX = xchAsks[md].begin();
    lv = 0;
    while (itX != xchAsks[md].end() && lv < PRINT_LV) {
        ++lv;
        PRICE_T price = itX->first;
        SIZE_T sz = itX->second;;
        cout << "ask " << std::setw(3) << std::setfill('0') << lv
            << std::fixed << std::setprecision(PRICE_SCALE)
            << " price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE) << endl; 
        ++itX;
    }
    cout << endl;
}

void OrderBook::printExtended() {
    std::lock_guard<std::recursive_mutex> lg(lock);
    cout << "BIDS" << endl;
    auto it = bids.begin();
    int lv = 0;
    while (it != bids.end() && lv < PRINT_LV) {
        ++lv;
        PRICE_T price = it->first;
        SIZE_T sz = getSize(true, price);
        SIZE_T binanceSz = it->second.contains(MktData::Binance) ?
            it->second[MktData::Binance] : 0;
        SIZE_T okxSz = it->second.contains(MktData::Okx) ?
            it->second[MktData::Okx] : 0;
        SIZE_T gateioSz = it->second.contains(MktData::GateIo) ?
            it->second[MktData::GateIo] : 0;
        cout << std::setw(3) << std::setfill('0') << lv
            << std::fixed << std::setprecision(PRICE_SCALE)
            << " price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE)
            << " -- "
            << "binance: " << scale_down(binanceSz, SIZE_SCALE)
            << ", okx: " << scale_down(okxSz, SIZE_SCALE)
            << ", gateio: " << scale_down(gateioSz, SIZE_SCALE) << endl; 
        ++it;
    }
    cout << "#" << endl;

    cout << "ASKS" << endl;
    it = asks.begin();
    lv = 0;
    while (it != asks.end() && lv < PRINT_LV) {
        ++lv;
        PRICE_T price = it->first;
        SIZE_T sz = getSize(false, price);
        SIZE_T binanceSz = it->second.contains(MktData::Binance) ?
            it->second[MktData::Binance] : 0;
        SIZE_T okxSz = it->second.contains(MktData::Okx) ?
            it->second[MktData::Okx] : 0;
        SIZE_T gateioSz = it->second.contains(MktData::GateIo) ?
            it->second[MktData::GateIo] : 0;
        cout << std::setw(3) << std::setfill('0') << lv
            << std::fixed << std::setprecision(PRICE_SCALE)
            << " price: " << scale_down(price, PRICE_SCALE)
            << std::setprecision(SIZE_SCALE)
            << ", size: " << scale_down(sz, SIZE_SCALE)
            << " -- "
            << "binance: " << scale_down(binanceSz, SIZE_SCALE)
            << ", okx: " << scale_down(okxSz, SIZE_SCALE)
            << ", gateio: " << scale_down(gateioSz, SIZE_SCALE) << endl; 
        ++it;
    }

    cout << endl;
    cout << "PER EXCHANGE:" << endl << endl;

    cout << "BINANCE" << endl;
    printXchBook(MktData::Binance);
    cout << "OKX" << endl;
    printXchBook(MktData::Okx);
    cout << "GATEIO" << endl;
    printXchBook(MktData::GateIo);
}

void OrderBook::clear() {
    std::lock_guard<std::recursive_mutex> lg(lock);
    bids.clear();
    asks.clear();
}

PRICE_T OrderBook::getMidPrice() {
    std::lock_guard<std::recursive_mutex> lg(lock);
    if (bids.empty() || asks.empty()) return 0;
    return (bids.begin()->first + asks.begin()->first) / 2;
}

}
}
