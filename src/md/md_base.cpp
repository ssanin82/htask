#include "md/md_base.h"

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef websocketpp::connection_hdl chdl;

using std::cout, std::endl, std::cerr, std::string, std::format;

namespace htask {
namespace md {

void MdBase::run() {
    while (true) {
        if (mCbReset) mCbReset();
        ws_client c;
        try {
            c.set_access_channels(websocketpp::log::alevel::all);
            c.clear_access_channels(
                websocketpp::log::alevel::frame_payload
            );
            c.init_asio();
            c.set_tls_init_handler([](chdl) {
                auto ctx = std::make_shared<boost::asio::ssl::context>(
                    boost::asio::ssl::context::tlsv12
                );
                ctx->set_default_verify_paths();
                return ctx;
            });
            c.set_open_handler([&](chdl hdl) {
                cout << mName << " connection opened" << endl;
                if (mSubsGenerate) {
                    websocketpp::lib::error_code ec;
                    string subs = mSubsGenerate();
                    c.send(hdl, subs, websocketpp::frame::opcode::text, ec);
                    if (ec) {
                        throw std::runtime_error(
                            format(
                                "{} subscription error: {}",
                                mName,
                                ec.message())
                        );
                    }
                }
            });
            c.set_fail_handler([&](chdl) {
                throw std::runtime_error(format("{} connection failed", mName));
            });
            c.set_close_handler([&](chdl) {
                throw std::runtime_error(format("{} connection closed", mName));
            });
            c.set_message_handler([&](chdl, ws_client::message_ptr msg
            ) {
                // cout << mName << " received message: "
                //     << msg->get_payload() << endl;
                mCbMsg(msg->get_payload());
            });
            websocketpp::lib::error_code ec;
            ws_client::connection_ptr con = c.get_connection(mUrl, ec);
            if (ec) {
                cout << "Could not create connection: " << ec.message() << endl;
                continue;
            }
            c.connect(con);
            c.run();
        } catch (websocketpp::exception const& e) {
            cout << "WebSocket++ exception: " << e.what() << endl;
        } catch (...) {
            cerr << "Caught unknown error" << endl;
            cout << "Reconnecting..." << endl;
        }
    }
}

}
}
