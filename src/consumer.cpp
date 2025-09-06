#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>
#include "proto/pubsub.grpc.pb.h"
#include "argparse/argparse.hpp"

#include "util.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using pubsub::PubSubService;
using pubsub::SubscriptionRequest;
using pubsub::Message;
using std::cout, std::endl, std::string;

using namespace htask::util;

class Subscriber {
    std::unique_ptr<PubSubService::Stub> stub;
public:
    Subscriber(std::shared_ptr<Channel> channel):
        stub(PubSubService::NewStub(channel)) {}

    void Subscribe(const string& topic) {
        SubscriptionRequest req;
        req.set_topic(topic);

        ClientContext context;
        std::unique_ptr<grpc::ClientReader<Message>> reader(
            stub->Subscribe(&context, req)
        );

        Message msg;
        while (reader->Read(&msg)) {
            // cout << "[Subscriber] Received on " << msg.topic()
            //     << ": " << msg.DebugString() << endl;
            uint64_t ts_ms = msg.publish_ts_ms();
            std::time_t time_t_now = ts_ms / 1000;
            std::tm tm_now = *std::localtime(&time_t_now);
            if (msg.topic() == "bba") {
                uint32_t bbp = msg.bba().best_bid_price();
                uint32_t bbs = msg.bba().best_bid_size();
                uint32_t bap = msg.bba().best_ask_price();
                uint32_t bas = msg.bba().best_ask_size();
                cout << "UPDATE ts: "
                    << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S")
                    << "." << (ts_ms % 1000) << endl;
                cout << "BEST BID: price="
                    << scale_down_to_str(bbp, PRICE_SCALE)
                    << ", size=" << scale_down_to_str(bbs, SIZE_SCALE) << endl;
                cout << "BEST ASK: price="
                    << scale_down_to_str(bap, PRICE_SCALE)
                    << ", size=" << scale_down_to_str(bas, SIZE_SCALE) << endl;
                cout << endl;
            } else if (msg.topic() == "vbd") {
                // move the received data to std::map to organize the
                // keys into a sorted order
                std::map<uint32_t, uint32_t> mbuys;
                std::map<uint32_t, uint32_t> msells;
                for (const auto& p: msg.vbd().mln_buy_price()) {
                    mbuys[p.first] = p.second;
                }
                for (const auto& p: msg.vbd().mln_sell_price()) {
                    msells[p.first] = p.second;
                }
                for (const auto& [val, buy_price]: mbuys) {
                    string buy_str = buy_price ?
                        ("USDT $" + scale_down_to_str(buy_price, PRICE_SCALE))
                        : "INSUFFICIENT LIQUIDITY";
                    cout << "Notional of USDT " << val
                        << " mln can be bought at: " << buy_str << endl;

                    uint32_t sell_price = msells[val];
                    string sell_str = sell_price ?
                        ("USDT $" + scale_down_to_str(sell_price, PRICE_SCALE))
                        : "INSUFFICIENT LIQUIDITY";
                    cout << "Notional of USDT " << val
                        << " mln can be sold at: " << sell_str << endl << endl;
                }
                cout << endl;
            } else { // pbd
                cout << "Mid price: "
                    << scale_down_to_str(msg.pbd().mid(), PRICE_SCALE) << endl;
                // move the received data to std::map to organize the
                // keys into a sorted order
                std::map<uint32_t, uint32_t> pups;
                std::map<uint32_t, uint32_t> pdowns;
                for (const auto& p: msg.pbd().bbo_up_bps()) {
                    pups[p.first] = p.second;
                }
                for (const auto& p: msg.pbd().bbo_down_bps()) {
                    pdowns[p.first] = p.second;
                }
                for (const auto& [bps, up_price]: pups) {
                    cout << bps << " bps price range: "
                        << scale_down_to_str(pdowns[bps], PRICE_SCALE)
                        << " - " << scale_down_to_str(up_price, PRICE_SCALE)
                        << endl;
                }
                cout << endl;
            }
        }

        Status status = reader->Finish();
        if (!status.ok()) {
            std::cerr << "Subscribe stream ended: "
                << status.error_message() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("pricer");
    program.add_argument("--host")
        .help("Server host")
        .default_value(std::string("localhost"));
    program.add_argument("--topic")
        .help("Topic name")
        .default_value(std::string("bba"));
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
    std::string topic = program.get<std::string>("--topic");
    int port = program.get<int>("--port");

    Subscriber sub(grpc::CreateChannel(
        std::format("{}:{}", host, port), grpc::InsecureChannelCredentials()
    ));
    sub.Subscribe(topic);
}
