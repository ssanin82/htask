#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "proto/pubsub.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using pubsub::PubSubService;
using pubsub::SubscriptionRequest;
using pubsub::PublishRequest;
using pubsub::PublishReply;
using pubsub::Message;
using std::cout, std::endl;

class PubSubServiceImpl final : public PubSubService::Service {
private:
    std::mutex subsLock;
    std::map<std::string, std::vector<ServerWriter<Message>*>> subscribers;

public:
    Status Subscribe(
        ServerContext* ctx,
        const SubscriptionRequest* req,
        ServerWriter<Message>* writer
    ) override {
        std::string topic = req->topic();
        {
            cout << "Subscribe request, topic: " << req->topic() << endl;
            std::lock_guard<std::mutex> lock(subsLock);
            subscribers[topic].push_back(writer);
        }
        while (!ctx->IsCancelled()) {
            // Block forever until client disconnects
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return Status::OK;
    }

    Status Publish(
        ServerContext* ctx,
        const PublishRequest* req,
        PublishReply* reply
    ) override {
        std::lock_guard<std::mutex> lock(subsLock);
        auto it = subscribers.find(req->topic());
        if (it != subscribers.end()) {
            for (auto writer : it->second) {
                Message msg;
                msg.set_topic(req->topic());
                msg.set_publish_ts_ms(req->publish_ts_ms());
                if (req->has_bba())  *msg.mutable_bba() = req->bba();
                else if (req->has_vbd()) *msg.mutable_vbd() = req->vbd();
                else *msg.mutable_pbd() = req->pbd();
                cout << "Forwarding: " << msg.DebugString() << endl;
                writer->Write(msg);
            }
        }
        reply->set_success(true);
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    PubSubServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
