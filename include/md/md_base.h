#pragma once

#include <string>
#include <functional>

#include <boost/asio/ssl/context.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

namespace htask {
namespace md {

class MdBase {
    std::string mName;
    std::string mUrl;
    std::function<void(const std::string& msg)> mCbMsg;
    std::function<std::string()> mSubsGenerate;
    std::function<void()> mCbReset;
public:
    MdBase(
        const std::string& name,
        const std::string& url,
        std::function<void(const std::string& msg)> on_msg,
        std::function<std::string()> on_subs_gen = nullptr,
        std::function<void()> on_reset = nullptr
    ): mName(name), mCbMsg(on_msg), mSubsGenerate(on_subs_gen),
    mCbReset(on_reset), mUrl(url) {}

    void run();
};

}
}
