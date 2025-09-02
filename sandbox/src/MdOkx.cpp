#include <iostream>
#include <string>
#include <ixwebsocket/IXWebSocket.h>

const std::string URL = "wss://ws.okx.com:8443/ws/v5/public";
const std::string SUBS_MSG = R"({
    "id": "1512",
    "op": "subscribe",
    "args": [
        {
            "channel": "books5",
            "instId": "BTC-USDT"
        }
    ]
})";

int main() {
    ix::WebSocket webSocket;

    webSocket.setUrl(URL);

    // Set up event handlers
    webSocket.setOnMessageCallback([&webSocket](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "Received message: " << msg->str << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Connection opened" << std::endl;
            webSocket.send(SUBS_MSG);
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

    return 0;
}