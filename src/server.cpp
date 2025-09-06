#include "server.h"

using std::cout, std::endl;

namespace htask {
namespace server {

// SubscriberRegistry ---------------------------------------------------

void SubscriberRegistry::Add(
    const std::string& topic, SubscriberSession* session
) {
    std::lock_guard<std::mutex> lock(mLock);
    mSubs[topic].insert(session);
}

void SubscriberRegistry::Remove(
    const std::string& topic, SubscriberSession* session
) {
    std::lock_guard<std::mutex> lock(mLock);
    auto it = mSubs.find(topic);
    if (it != mSubs.end()) {
        it->second.erase(session);
        if (it->second.empty()) {
            mSubs.erase(it);
        }
    }
}

void SubscriberRegistry::Publish(const Message& msg) {
    std::lock_guard<std::mutex> lock(mLock);
    auto it = mSubs.find(msg.topic());
    if (it == mSubs.end()) return;

    for (auto* s : it->second) {
        s->SendMessage(msg);
    }
}

// SubscriberSession ---------------------------------------------------

SubscriberSession::SubscriberSession(
    PubSubService::AsyncService* service, ServerCompletionQueue* cq
): mService(service), mCq(cq), mResponder(&mCtx), mStatus(CallStatus::CREATE) {
    cout << "New subscriber session pre-created." << endl;
    Proceed(true);
}

void SubscriberSession::Proceed(bool ok) {
    if (mStatus == CallStatus::CREATE) {
        mStatus = CallStatus::PROCESS;
        mService->RequestSubscribe(
            &mCtx, &mRequest, &mResponder, mCq, mCq, this
        );
    } else if (mStatus == CallStatus::PROCESS) {
        cout << "SubscriberSession:PROCESS" << endl;
        // Spawn a new handler for next subscriber
        new SubscriberSession(mService, mCq);
        cout << "New subscriber on topic: " << mRequest.topic() << endl;
        mTopic = mRequest.topic();
        SubscriberRegistry::getInstance().Add(mTopic, this);
    } else if (mStatus == CallStatus::WRITING) {
        cout << "SubscriberSession:WRITING" << endl;
        if (!ok) {
            HandleDisconnect();
            return;
        }
        mStatus = CallStatus::PROCESS; // ready for next write
    } else {
        delete this;
    }
}

void SubscriberSession::SendMessage(const Message& msg) {
    if (mStatus != CallStatus::PROCESS) return;
    mStatus = CallStatus::WRITING;
    mResponder.Write(msg, this); // async write
}

void SubscriberSession::OnWriteDone(bool ok) {
    if (!ok) {
        cout << "subscriber disconnected" << endl;
        HandleDisconnect();
    }
    // otherwise ready for next write
    cout << "subscriber: proceed" << endl;
}

void SubscriberSession::HandleDisconnect() {
    cout << "Subscriber disconnected from topic: " << mTopic << endl;
    SubscriberRegistry::getInstance().Remove(mTopic, this);
    mStatus = CallStatus::FINISH;
    Proceed(true);
}

// PublishCall ---------------------------------------------------

PublishCall::PublishCall(
    PubSubService::AsyncService* service,
    ServerCompletionQueue* cq
): mService(service), mCq(cq), mResponder(&mCtx) {
    mStatus = CallStatus::CREATE;
    Proceed(true);
}

void PublishCall::Proceed(bool ok) {
    if (mStatus == CallStatus::CREATE) {
        mStatus = CallStatus::PROCESS;
        mService->RequestPublish(
            &mCtx, &mRequest, &mResponder, mCq, mCq, this
        );
    } else if (mStatus == CallStatus::PROCESS) {
        // Spawn next publisher handler
        new PublishCall(mService, mCq);
        cout << "Publishing: " << mRequest.DebugString() << endl;
        SubscriberRegistry::getInstance().Publish(mRequest);
        mReply.set_success(true);
        mStatus = CallStatus::FINISH;
        mResponder.Finish(mReply, Status::OK, this);
    } else delete this;
}

// ServerImpl ---------------------------------------------------

ServerImpl::~ServerImpl() {
    mServer->Shutdown();
    mCq->Shutdown();
}

void ServerImpl::HandleRpcs() {
    void* tag;
    bool ok;
    while (mCq->Next(&tag, &ok)) {
        cout << "Received an item in completion queue" << endl;
        static_cast<CallData*>(tag)->Proceed(ok);
        cout << "Completion queue item processed" << endl;
    }
}

void ServerImpl::Run(const std::string& addr) {
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&mService);
    mCq = builder.AddCompletionQueue();
    mServer = builder.BuildAndStart();
    cout << "Server listening on " << addr << endl;

    new SubscriberSession(&mService, mCq.get());
    new PublishCall(&mService, mCq.get());

    HandleRpcs();
}

}
}

int main() {
    htask::server::ServerImpl server;
    server.Run("0.0.0.0:50051");
    return 0;
}
