#include <iostream>
#include <string>

#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

namespace htask {
namespace md_okx {

const string URL = "wss://ws.okx.com:8443/ws/v5/public";
const string SUBS_MSG = R"({
    "id": "1512",
    "op": "subscribe",
    "args": [
        {
            "channel": "books",
            "instId": "BTC-USDT"
        }
    ]
})";

void processMsg(OrderBook& ob, const string& msg) {
    json j = nlohmann::json::parse(msg);
    for (auto& d: j["data"]) {
        for (const auto& lv: d["bids"]) {
            ob.updateLevel(MktData::Okx, true, lv[0], lv[1]);
        }
        for (const auto& lv: d["asks"]) {
            ob.updateLevel(MktData::Okx, false, lv[0], lv[1]);
        }
    }
    ob.print();
}

void work(OrderBook& ob) {
    while (true) {
        try {
            ix::WebSocket ws;
            ws.setUrl(URL);
            ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message) {
                    // cout << "Received message: " << msg->str << endl;
                    processMsg(ob, msg->str);
                } else if (msg->type == ix::WebSocketMessageType::Open) {
                    cout << "OKX connection opened" << endl;
                    ws.send(SUBS_MSG);
                } else if (msg->type == ix::WebSocketMessageType::Error) {
                    cerr << "OKX error: " << msg->errorInfo.reason << endl;
                } else if (msg->type == ix::WebSocketMessageType::Close) {
                    cout << "OKX connection closed" << endl;
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
    }
    // ws.stop();
}

}
}
