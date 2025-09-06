#include <iostream>
#include <thread>
#include <chrono>

#include "OrderBook.h"
#include "md/binance.h"
#include "md/okx.h"
#include "md/gateio.h"

#include <grpcpp/grpcpp.h>
#include "proto/pubsub.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using pubsub::PubSubService;
using pubsub::PublishRequest;
using pubsub::PublishReply;
using pubsub::BbaData;
using pubsub::VolBandsData;
using pubsub::PriceBandsData;
using namespace htask::util;

constexpr int PUB_IVAL_SEC = 1;
constexpr int PRINT_IVAL_SEC = 5;
constexpr const char* PUB_ADDR = "0.0.0.0:5051";

class Publisher {
    std::unique_ptr<PubSubService::Stub> stub;
public:
    Publisher(std::shared_ptr<Channel> channel):
        stub(PubSubService::NewStub(channel)) {}

    void Publish(const PublishRequest& req) {
        PublishReply reply;
        ClientContext ctx;
        Status status = stub->Publish(&ctx, req, &reply);
        if (status.ok() && reply.success()) {
            std::cout << "[Publisher] Sent: "
                << req.DebugString()
                << " to topic " << req.topic() << std::endl;
        } else std::cerr << "Publish failed." << std::endl;
    }
};

void publish(OrderBook& ob, Publisher& pub) {
    auto bbid = ob.getBestBid();
    auto bask = ob.getBestAsk();
    BbaData bba;
    bba.set_best_bid_price(bbid.first);
    bba.set_best_bid_size(bbid.second);
    bba.set_best_ask_price(bask.first);
    bba.set_best_ask_size(bask.second);
    PublishRequest requestBba;
    requestBba.set_topic("bba");
    *requestBba.mutable_bba() = bba;
    pub.Publish(requestBba);

    VolBandsData vbd;
    vbd.mutable_mln_buy_price()->insert({1, ob.getVolumePriceMln(false, 1)});
    vbd.mutable_mln_sell_price()->insert({1, ob.getVolumePriceMln(true, 1)});
    vbd.mutable_mln_buy_price()->insert({5, ob.getVolumePriceMln(false, 5)});
    vbd.mutable_mln_sell_price()->insert({5, ob.getVolumePriceMln(true, 5)});
    vbd.mutable_mln_buy_price()->insert({10, ob.getVolumePriceMln(false, 10)});
    vbd.mutable_mln_sell_price()->insert({10, ob.getVolumePriceMln(true, 10)});
    vbd.mutable_mln_buy_price()->insert({25, ob.getVolumePriceMln(false, 25)});
    vbd.mutable_mln_sell_price()->insert({25, ob.getVolumePriceMln(true, 25)});
    vbd.mutable_mln_buy_price()->insert({50, ob.getVolumePriceMln(false, 50)});
    vbd.mutable_mln_sell_price()->insert({50, ob.getVolumePriceMln(true, 50)});
    PublishRequest requestVbd;
    requestBba.set_topic("vbd");
    *requestBba.mutable_vbd() = vbd;
    pub.Publish(requestVbd);

    PriceBandsData pbd;
    PRICE_T mid = (bbid.first + bask.first) / 2;
    pbd.set_bbo(mid);
    pbd.mutable_bbo_up_bps()->insert({50, static_cast<int>(mid * (1.005))});
    pbd.mutable_bbo_down_bps()->insert({50, static_cast<int>(mid * (0.995))});
    pbd.mutable_bbo_up_bps()->insert({100, static_cast<int>(mid * (1.01))});
    pbd.mutable_bbo_down_bps()->insert({100, static_cast<int>(mid * (0.99))});
    pbd.mutable_bbo_up_bps()->insert({200, static_cast<int>(mid * (1.02))});
    pbd.mutable_bbo_down_bps()->insert({200, static_cast<int>(mid * (0.98))});
    pbd.mutable_bbo_up_bps()->insert({500, static_cast<int>(mid * (1.05))});
    pbd.mutable_bbo_down_bps()->insert({500, static_cast<int>(mid * (0.95))});
    pbd.mutable_bbo_up_bps()->insert({1000, static_cast<int>(mid * (1.1))});
    pbd.mutable_bbo_down_bps()->insert({1000, static_cast<int>(mid * (0.9))});
    PublishRequest requestPbd;
    requestBba.set_topic("pbd");
    *requestBba.mutable_pbd() = pbd;
    pub.Publish(requestPbd);
}

int main() {
    htask::util::OrderBook ob;

    std::jthread tBinance(htask::md_binance::work, std::ref(ob));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // XXX binance needs some time to synchronize initial order book
    // snapshit with the buffered updates

    std::jthread tGateIo(htask::md_gateio::work, std::ref(ob));
    std::jthread tOkx(htask::md_okx::work, std::ref(ob));

    Publisher pub(grpc::CreateChannel(
        PUB_ADDR, grpc::InsecureChannelCredentials()
    ));

    uint32_t counter = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++counter;
        // if (0 == counter % PUB_IVAL_SEC) publish(ob, pub);
        if (0 == counter % PRINT_IVAL_SEC) {
            ob.print();
            // ob.printExtended();
        }
    }
    return 0;
}
