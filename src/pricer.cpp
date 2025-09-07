#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <condition_variable>

#include "OrderBook.h"
#include "md/binance.h"
#include "md/okx.h"
#include "md/gateio.h"

#include <grpcpp/grpcpp.h>
#include "proto/pubsub.grpc.pb.h"
#include "argparse/argparse.hpp"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using pubsub::PubSubService;
using pubsub::Message;
using pubsub::SubscriptionRequest;
using pubsub::BbaData;
using pubsub::VolBandsData;
using pubsub::PriceBandsData;
using namespace htask::util;
using std::cout, std::endl;

constexpr int PUB_IVAL_SEC = 1;
constexpr int PRINT_IVAL_SEC = 5;

class SubscriberQueue {
    std::mutex mLock;
    std::queue<Message> mQ;
    std::condition_variable mCv;
public:
    void Push(const Message& msg) {
        std::unique_lock<std::mutex> lock(mLock);
        mQ.push(msg);
        mCv.notify_one();
    }

    bool Pop(Message& msg, ServerContext* context) {
        std::unique_lock<std::mutex> lock(mLock);
        while (mQ.empty()) {
            if (context->IsCancelled()) {
                cout << "Client disconnected, topic: " << msg.topic() << endl;
                return false;
            }
            mCv.wait_for(lock, std::chrono::milliseconds(100));
        }
        msg = mQ.front();
        mQ.pop();
        return true;
    }
};

class PubSubServiceImpl final : public PubSubService::Service {
    std::mutex mLock;
    std::map<std::string, std::vector<std::shared_ptr<SubscriberQueue>>> mSubs;
public:
    Status Subscribe(ServerContext* context,
                     const SubscriptionRequest* request,
                     ServerWriter<Message>* writer) override {
        std::string topic = request->topic();
        auto queue = std::make_shared<SubscriberQueue>();
        {
            std::unique_lock<std::mutex> lock(mLock);
            mSubs[topic].push_back(queue);
        }
        cout << "New subscriber on topic: " << topic << endl;
        Message msg;
        while (queue->Pop(msg, context)) writer->Write(msg);
        cout << "Subscriber disconnected from topic: " << topic << endl;
        return Status::OK;
    }

    void Publish(const Message& msg) {
        std::unique_lock<std::mutex> lock(mLock);
        auto it = mSubs.find(msg.topic());
        if (it != mSubs.end()) for (auto& q: it->second) q->Push(msg);
    }
};

void publish(OrderBook& ob, PubSubServiceImpl& pub) {
    auto now = std::chrono::system_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    );
    time_t ts_ms = dur.count();

    auto bbid = ob.getBestBid();
    auto bask = ob.getBestAsk();
    BbaData bba;
    bba.set_best_bid_price(bbid.first);
    bba.set_best_bid_size(bbid.second);
    bba.set_best_ask_price(bask.first);
    bba.set_best_ask_size(bask.second);
    Message msgBba;
    msgBba.set_topic("bba");
    msgBba.set_publish_ts_ms(ts_ms);
    *msgBba.mutable_bba() = bba;
    pub.Publish(msgBba);

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
    Message msgVbd;
    msgVbd.set_topic("vbd");
    msgVbd.set_publish_ts_ms(ts_ms);
    *msgVbd.mutable_vbd() = vbd;
    pub.Publish(msgVbd);

    PriceBandsData pbd;
    PRICE_T mid = (bbid.first + bask.first) / 2;
    pbd.set_mid(mid);
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
    Message msgPbd;
    msgPbd.set_topic("pbd");
    msgPbd.set_publish_ts_ms(ts_ms);
    *msgPbd.mutable_pbd() = pbd;
    pub.Publish(msgPbd);
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("pricer");
    program.add_argument("--host")
        .help("Server host")
        .default_value(std::string("localhost"));
    program.add_argument("--port")
        .help("Server port")
        .scan<'i', int>() // parse as int
        .default_value(50051);
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program << std::endl;
        return 1;
    }

    std::string host = program.get<std::string>("--host");
    int port = program.get<int>("--port");

    htask::util::OrderBook ob;

    std::jthread tBinance(htask::md_binance::work, std::ref(ob));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // XXX binance needs some time to synchronize initial order book
    // snapshit with the buffered updates

    std::jthread tGateIo(htask::md_gateio::work, std::ref(ob));
    std::jthread tOkx(htask::md_okx::work, std::ref(ob));

    std::string pub_addr = std::format("{}:{}", host, port);
    PubSubServiceImpl pub;
    ServerBuilder builder;
    builder.AddListeningPort(pub_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&pub);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Publisher listening on " << pub_addr << std::endl;

    uint32_t counter = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++counter;
        if (0 == counter % PUB_IVAL_SEC) publish(ob, pub);
        if (0 == counter % PRINT_IVAL_SEC) {
            ob.print();
            // ob.printExtended();
        }
    }
    return 0;
}
