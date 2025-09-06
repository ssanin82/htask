#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <boost/asio/ssl/context.hpp>   // required for TLS
// #include <ixwebsocket/IXWebSocket.h>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
using websocketpp::connection_hdl;

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
        ob.print();
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
    ob.print();
}

// void _work(OrderBook& ob) {
//     while (true) {
//         try {
//             std::queue<json> initBuffer;
//             ix::WebSocket ws;
//             ws.setUrl(URL);
//             ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
//                 if (msg->type == ix::WebSocketMessageType::Message) {
//                     // cout << "Binance received message: " << msg->str << endl;
//                     json j = nlohmann::json::parse(msg->str);
//                     if (!init_lu) {
//                         initBuffer.push(std::move(j));
//                     } else {
//                         if (!initBuffer.empty()) {
//                             while (!initBuffer.empty()) {
//                                 auto jj = initBuffer.front();
//                                 initBuffer.pop();
//                                 if (jj["u"] > init_lu) processMsg(ob, jj);
//                             }
//                         }
//                         processMsg(ob, j);
//                     }
//                 } else if (msg->type == ix::WebSocketMessageType::Open) {
//                     cout << "Binance connection opened" << endl;
//                 } else if (msg->type == ix::WebSocketMessageType::Error) {
//                     cerr << "Binance error: " << msg->errorInfo.reason << endl;
//                 } else if (msg->type == ix::WebSocketMessageType::Close) {
//                     cout << "Binance connection closed" << endl;
//                 }
//             });
//             ws.start();
//             while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
//         } catch (const std::runtime_error& e) {
//             cerr << "Caught std::runtime_error: " << e.what() << endl;
//             cout << "Reconnecting..." << endl;
//         } catch (...) {
//             cerr << "Caught unknown error" << endl;
//             cout << "Reconnecting..." << endl;
//         }
//         init_lu = 0UL;
//     }
//     // ws.stop();
// }

void _work(OrderBook& ob) {
    while (true) {
        client c;
        try {
            std::queue<json> initBuffer;
            c.set_access_channels(websocketpp::log::alevel::all);
            c.clear_access_channels(websocketpp::log::alevel::frame_payload);
            c.init_asio();
            c.set_tls_init_handler([](connection_hdl) {
                auto ctx = std::make_shared<boost::asio::ssl::context>(
                    boost::asio::ssl::context::tlsv12
                );
                ctx->set_default_verify_paths();
                return ctx;
            });
            c.set_message_handler([&](connection_hdl, client::message_ptr msg) {
                cout << "Binance received message: "
                    << msg->get_payload() << endl;
                json j = nlohmann::json::parse(msg->get_payload());
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
            });
            websocketpp::lib::error_code ec;
            client::connection_ptr con = c.get_connection(URL, ec);
            if (ec) {
                cout << "Could not create connection: " << ec.message() << endl;
                continue;
            }
            c.connect(con);
            c.run();

        } catch (websocketpp::exception const& e) {
            cout << "WebSocket++ exception: " << e.what() << endl;
        } catch (...) {
            cerr << "Caught unknown error" << endl;
            cout << "Reconnecting..." << endl;
        }
    }
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
