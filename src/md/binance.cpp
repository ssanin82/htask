#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

namespace htask {
namespace md_binance {

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
    while (true) {
        try {
            std::queue<json> initBuffer;
            ix::WebSocket ws;
            ws.setUrl(URL);
            ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message) {
                    // cout << "Binance received message: " << msg->str << endl;
                    json j = nlohmann::json::parse(msg->str);
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
                } else if (msg->type == ix::WebSocketMessageType::Open) {
                    cout << "Binance connection opened" << endl;
                } else if (msg->type == ix::WebSocketMessageType::Error) {
                    cerr << "Binance error: " << msg->errorInfo.reason << endl;
                } else if (msg->type == ix::WebSocketMessageType::Close) {
                    cout << "Binance connection closed" << endl;
                }
            });
            ws.start();
            while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::runtime_error& e) {
            cerr << "Caught std::runtime_error: " << e.what() << endl;
            cout << "Reconnecting..." << endl;
        } catch (...) {
            cerr << "Caught unknown error" << endl;
            cout << "Reconnecting..." << endl;
        }
        init_lu = 0UL;
    }
    // ws.stop();
}

void work(OrderBook& ob) {
    std::jthread t(_work, std::ref(ob));
    // let market data start coming
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
