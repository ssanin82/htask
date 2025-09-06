#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "md/md_base.h"
#include "OrderBook.h"

using namespace htask::util;
using nlohmann::json;
using std::cout, std::endl, std::cerr, std::string;

namespace htask {
namespace md_okx {

const string NAME = "OKX";
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
    // ob.print();
}

void work(OrderBook& ob) {
    htask::md::MdBase md(
        NAME,
        URL,
        [&ob](const std::string& msg) { processMsg(ob, msg); },
        []() { return SUBS_MSG; }
    );
    md.run();
}

}
}
