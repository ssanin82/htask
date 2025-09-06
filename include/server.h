#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include "proto/pubsub.grpc.pb.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::ServerAsyncWriter;
using grpc::Status;

using pubsub::PubSubService;
using pubsub::SubscriptionRequest;
using pubsub::Message;
using pubsub::PublishReply;

namespace htask {
namespace server {

enum class CallStatus { CREATE, PROCESS, WRITING, FINISH };

struct CallData {
    virtual void Proceed(bool ok) = 0;
    virtual ~CallData() {}
};

class SubscriberSession;

class SubscriberRegistry {
    std::mutex mLock;
    std::unordered_map<std::string, std::unordered_set<SubscriberSession*>> mSubs;
public:
    void Add(const std::string& topic, SubscriberSession* session);
    void Remove(const std::string& topic, SubscriberSession* session);
    void Publish(const Message& msg);

    static SubscriberRegistry& getInstance() {
        static SubscriberRegistry instance;
        return instance;
    }
};

class SubscriberSession: public CallData {
    PubSubService::AsyncService* mService;
    ServerCompletionQueue* mCq;
    ServerContext mCtx;
    SubscriptionRequest mRequest;
    ServerAsyncWriter<Message> mResponder;

    std::string mTopic;
    CallStatus mStatus;
public:
    SubscriberSession(
        PubSubService::AsyncService* service,
        ServerCompletionQueue* cq
    );
    void Proceed(bool ok) override;
    void SendMessage(const Message& msg);
    void OnWriteDone(bool ok);
    void HandleDisconnect();
};

// Publisher request
class PublishCall: public CallData {
    PubSubService::AsyncService* mService;
    ServerCompletionQueue* mCq;
    ServerContext mCtx;
    Message mRequest;
    PublishReply mReply;
    grpc::ServerAsyncResponseWriter<PublishReply> mResponder;
    CallStatus mStatus;
public:
    PublishCall(
        PubSubService::AsyncService* service, ServerCompletionQueue* cq
    );
    void Proceed(bool ok) override;
};

class ServerImpl {
    std::unique_ptr<ServerCompletionQueue> mCq;
    PubSubService::AsyncService mService;
    std::unique_ptr<Server> mServer;

    void HandleRpcs();
public:
    ~ServerImpl();
    void Run(const std::string& addr);
};

}
}
