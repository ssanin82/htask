#include <iostream>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "proto/pubsub.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using pubsub::PubSubService;
using pubsub::SubscriptionRequest;
using pubsub::Message;

constexpr const char* SUB_ADDR = "0.0.0.0:5051";

class Subscriber {
    std::unique_ptr<PubSubService::Stub> stub;
public:
    Subscriber(std::shared_ptr<Channel> channel):
        stub(PubSubService::NewStub(channel)) {}

    void Subscribe(const std::string& topic) {
        SubscriptionRequest req;
        req.set_topic(topic);

        ClientContext context;
        std::unique_ptr<grpc::ClientReader<Message>> reader(
            stub->Subscribe(&context, req)
        );

        Message msg;
        while (reader->Read(&msg)) {
            std::cout << "[Subscriber] Received on " << msg.topic()
                << ": " << msg.DebugString() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./consumer <topic>" << std::endl;
        return -1;
    }
    std::string topic(argv[1]);
    if (topic != "bbo" && topic != "vbd" && topic != "pbd") {
        std::cout << "Invalid topic [" << topic << "]!" << std::endl;
        std::cout << "Supported topics: bba, vbd, pbd" << std::endl;
        return -1;
    }

    Subscriber sub(grpc::CreateChannel(
        SUB_ADDR, grpc::InsecureChannelCredentials()
    ));
    sub.Subscribe(topic);
}
