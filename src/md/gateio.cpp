#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>

#include "md/md_base.h"
#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

namespace htask {
namespace md_gateio {

const string NAME = "GATEIO";
const string URL = "wss://api.gateio.ws/ws/v4/";

void processMsg(OrderBook& ob, const string& msg) {
    json j = nlohmann::json::parse(msg);
    for (const auto& lv: j["result"]["b"]) {
        ob.updateLevel(MktData::GateIo, true, lv[0], lv[1]);
    }
    for (const auto& lv: j["result"]["a"]) {
        ob.updateLevel(MktData::GateIo, false, lv[0], lv[1]);
    }
    // ob.print();
}

void work(OrderBook& ob) {
    htask::md::MdBase md(
        NAME,
        URL,
        [&ob](const std::string& msg) { processMsg(ob, msg); },
        []() {
            json j;
            j["time"] = time(0);
            j["channel"] = "spot.obu";
            j["event"] = "subscribe";
            j["payload"] = {"ob.BTC_USDT.400"};
            return j.dump();
        }
    );
    md.run();
}

}
}
