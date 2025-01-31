#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include "pld_handler.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
public:
    WebSocketClient() {
        // Inicialize o WebSocket++ client
        c.init_asio();

        // Configure os callbacks
        c.set_open_handler([this](websocketpp::connection_hdl hdl) {
            on_open(hdl);
        });
        c.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            on_message(hdl, msg);
        });
        c.set_fail_handler([this](websocketpp::connection_hdl hdl) {
            std::cerr << "Connection failed" << std::endl;
        });
        c.set_close_handler([this](websocketpp::connection_hdl hdl) {
            on_close(hdl);
        });
    }

    void connect(const std::string& uri) {
        websocketpp::lib::error_code ec;
        auto con = c.get_connection(uri, ec);

        if (ec) {
            std::cerr << "Connection error: " << ec.message() << std::endl;
            return;
        }

        c.connect(con);

        // Inicia o loop de execução do WebSocket
        c.run();
    }

private:
    void on_open(websocketpp::connection_hdl hdl) {
        std::cout << "Connected!" << std::endl;
    }

    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::string payload = msg->get_payload();

        process_payload(payload);
    }

    void on_close(websocketpp::connection_hdl hdl) {
        std::cout << "Connection closed" << std::endl;
    }

    client c;
};

int main() {
    std::string url = "ws://192.168.1.19:8080/sensors/connect?types=[\"android.sensor.accelerometer\",\"android.sensor.gyroscope\",\"android.sensor.magnetic_field\"]";

    WebSocketClient wsClient;
    wsClient.connect(url);

    return 0;
}
