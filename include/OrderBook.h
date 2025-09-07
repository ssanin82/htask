#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <mutex>

#include "util.h"

namespace htask {
namespace util {

enum class MktData {
    Binance,
    Okx,
    GateIo
};

class OrderBook {
        std::map<PRICE_T, std::unordered_map<MktData, SIZE_T>, std::greater<PRICE_T>> bids;
        std::map<PRICE_T, std::unordered_map<MktData, SIZE_T>> asks;
        std::unordered_map<MktData, std::map<PRICE_T, SIZE_T, std::greater<PRICE_T>>> xchBids;
        std::unordered_map<MktData, std::map<PRICE_T, SIZE_T>> xchAsks;

        std::pair<PRICE_T, SIZE_T> getBest(bool isBid);
        SIZE_T getSize(bool isBid, PRICE_T price);

        void printXchBook(MktData md);

        std::recursive_mutex lock;
    public:
        void updateLevel(
            MktData md,
            bool isBid,
            const std::string& price,
            const std::string& size
        );

        std::pair<PRICE_T, SIZE_T> getBestBid();
        std::pair<PRICE_T, SIZE_T> getBestAsk();

        PRICE_T getVolumePrice(bool isBid, SIZE_T target_usdt);
        PRICE_T getVolumePriceMln(bool isBid, int x);
        PRICE_T getMidPrice();

        void print();
        void printExtended();
        void clear();
};

}
}
