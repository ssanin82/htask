#pragma once

#include <map>
#include <string>
#include <unordered_map>

namespace htask {
namespace util {

enum class MktData {
    Binance,
    Okx,
    GateIo
};

class OrderBook {
        std::map<std::string, std::unordered_map<MktData, std::string>, std::greater<std::string>> bids;
        std::map<std::string, std::unordered_map<MktData, std::string>> asks;

        std::pair<std::string, std::string> getBest(bool isBid);
        std::string getSize(bool isBid, const std::string& price);
    public:
        void updateLevel(
            MktData md,
            bool isBid,
            const std::string& price,
            const std::string& size
        );

        std::pair<std::string, std::string> getBestBid();
        std::pair<std::string, std::string> getBestAsk();

        double getVolumePrice(bool isBid, double target);
        double getVolumePriceMln(bool isBid, int x);

        void print();
        void clear();
};

}
}
