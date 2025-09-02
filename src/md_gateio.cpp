#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

namespace htask {
namespace md_gateio {

const string URL = "wss://api.gateio.ws/ws/v4/";

void processMsg(OrderBook& ob, const string& msg) {
    json j = nlohmann::json::parse(msg);
    for (const auto& lv: j["result"]["b"]) {
        ob.updateLevel(MktData::GateIo, true, lv[0], lv[1]);
    }
    for (const auto& lv: j["result"]["a"]) {
        ob.updateLevel(MktData::GateIo, false, lv[0], lv[1]);
    }
    ob.print();
}

void work() {
    while (true) {
        try {
            OrderBook ob;
            ix::WebSocket ws;
            ws.setUrl(URL);
            ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message) {
                    cout << "Received message: " << msg->str << endl;
                    processMsg(ob, msg->str);
                } else if (msg->type == ix::WebSocketMessageType::Open) {
                    cout << "Gate.io connection opened" << endl;
                    json j;
                    j["time"] = time(0);
                    j["channel"] = "spot.obu";
                    j["event"] = "subscribe";
                    j["payload"] = {"ob.BTC_USDT.400"};
                    ws.send(j.dump());
                } else if (msg->type == ix::WebSocketMessageType::Error) {
                    cerr << "Gate.io error: " << msg->errorInfo.reason << endl;
                } else if (msg->type == ix::WebSocketMessageType::Close) {
                    cout << "Gate.io connection closed" << endl;
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
