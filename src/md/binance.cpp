#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <boost/asio/ssl/context.hpp>   // required for TLS
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "md/md_base.h"
#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
using websocketpp::connection_hdl;

namespace htask {
namespace md_binance {

const string NAME = "BINANCE";
const std::string URL = "wss://stream.binance.com:9443/ws/btcusdt@depth@100ms";
const std::string OB_REQ = "https://api.binance.com/api/v3/depth?symbol=BTCUSDT&limit=5000";
uint64_t init_lu = 0UL;

uint64_t getInitOb(OrderBook& ob) {
    cpr::Response r = cpr::Get(cpr::Url{OB_REQ});
    try {
        json j = nlohmann::json::parse(r.text);
        for (const auto& lv: j["bids"]) {
            ob.updateLevel(MktData::Binance, true, lv[0], lv[1]);
        }
        for (const auto& lv: j["asks"]) {
            ob.updateLevel(MktData::Binance, false, lv[0], lv[1]);
        }
        // ob.print();
        return j["lastUpdateId"];
    } catch (...) {
        cerr << "Binance - ERROR parsing: " << r.text << endl;
        throw;
    }
}

void processMsg(OrderBook& ob, const json& j) {
    for (const auto& lv: j["b"]) {
        ob.updateLevel(MktData::Binance, true, lv[0], lv[1]);
    }
    for (const auto& lv: j["a"]) {
        ob.updateLevel(MktData::Binance, false, lv[0], lv[1]);
    }
    // ob.print();
}

void _work(OrderBook& ob) {
    std::queue<json> initBuffer;
    htask::md::MdBase md(
        NAME,
        URL,
        [&](const std::string& msg) {
            json j = nlohmann::json::parse(msg);
            if (!init_lu) {
                initBuffer.push(std::move(j));
            } else {
                if (!initBuffer.empty()) {
                    while (!initBuffer.empty()) {
                        auto jj = initBuffer.front();
                        initBuffer.pop();
                        if (jj["u"] > init_lu) processMsg(ob, jj);
                    }
                }
                processMsg(ob, j);
            }
        },
        nullptr,
        [&initBuffer]() { while(!initBuffer.empty()) initBuffer.pop(); }
    );
    md.run();
}

void work(OrderBook& ob) {
    std::jthread t(_work, std::ref(ob));
    // XXX hack: allow some time for market data to start coming
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (true) {
        // check if it is a first start or a restart
        if (!init_lu) {
            ob.clear();
            cout << "Binance: requesting initial ob snapshot" << endl;
            init_lu = getInitOb(ob);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

}
}
