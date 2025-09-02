#pragma once

#include <map>
#include <string>

namespace htask {
namespace util {

class OrderBook {
        std::map<std::string, std::string, std::greater<std::string>> bids;
        std::map<std::string, std::string> asks;
    public:
        void updateLevel(
            bool isBid, const std::string& price, const std::string& size
        );

        std::pair<std::string, std::string> getBestBid();

        void print();
        void clear();
};

}
}
