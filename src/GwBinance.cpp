
#include <iostream>
#include <string>
#include <ixwebsocket/IXWebSocket.h>

namespace htask {
namespace md_binance {

void worker() {
    ix::WebSocket webSocket;
    webSocket.setUrl("wss://stream.binance.com:9443/ws/btcusdt@depth5");
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "Received message: " << msg->str << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Connection opened" << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "Connection closed" << std::endl;
        }
    });

    // Start the connection
    webSocket.start();

    // Send messages
    while (true) {
        std::cout << "Enter message to send (or 'exit' to quit): ";
        std::string message;
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        webSocket.send(message);
    }

    // Stop the connection
    webSocket.stop();
}

}
}
