#include <iostream>
#include <string>

#include <ixwebsocket/IXWebSocket.h>

using std::cout, std::endl, std::cerr, string;

namespace htask {
namespace md_okx {

const string URL = "wss://ws.okx.com:8443/ws/v5/public";
const string SUBS_MSG = R"({
    "id": "1512",
    "op": "subscribe",
    "args": [
        {
            "channel": "books5",
            "instId": "BTC-USDT"
        }
    ]
})";

void processMsg(OrderBook& ob, const string& msg) {
    json j = nlohmann::json::parse(msg);
    for (const auto& lv: j["result"]["b"]) ob.updateLevel(true, lv[0], lv[1]);
    for (const auto& lv: j["result"]["a"]) ob.updateLevel(false, lv[0], lv[1]);
    ob.print();
}

int main() {
    while (true) {
        try {
            ix::WebSocket ws;
            ws.setUrl(URL);
            ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message) {
                    cout << "Received message: " << msg->str << endl;
                    // processMsg(ob, msg->str);
                } else if (msg->type == ix::WebSocketMessageType::Open) {
                    cout << "Connection opened" << endl;
                    ws.send(SUBS_MSG);
                } else if (msg->type == ix::WebSocketMessageType::Error) {
                    cerr << "Error: " << msg->errorInfo.reason << endl;
                } else if (msg->type == ix::WebSocketMessageType::Close) {
                    cout << "Connection closed" << endl;
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
