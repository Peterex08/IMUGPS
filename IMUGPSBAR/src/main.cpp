#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <cmath>
#include "../include/payload_handler.h"

using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// C++ é genuinamente um inferno!
const std::vector<std::string> types_order = {
    "android.sensor.accelerometer",
    "android.sensor.gyroscope",
    "android.sensor.magnetic_field",
    "android.sensor.pressure",
    "gps"
};

std::vector<json> collected_values(types_order.size());
int ctrl = 0;
double lat = 0, lon = 0, alt = 0;
long long timestamp = 0;  

class Sensor {
public:
    using websocket_client = websocketpp::client<websocketpp::config::asio_client>;
    
    Sensor(const std::string& address, const std::string& sensor_type,
           std::function<void(const json&)> callback);
    
    void connect();
    void send_requests(const std::string& message);

private:
    void on_message(websocketpp::connection_hdl, websocket_client::message_ptr msg);
    void on_open(websocketpp::connection_hdl);
    void on_fail(websocketpp::connection_hdl);
    void on_close(websocketpp::connection_hdl);

    websocket_client endpoint_;
    websocket_client::connection_ptr connection_;
    std::string address_;
    std::string sensor_type_;
    std::function<void(const json&)> callback_;
};

// Global sensor instances
std::unique_ptr<Sensor> imu_sensor;
std::unique_ptr<Sensor> gps_sensor;

void calc_vel(const std::vector<json>& arr) {
    const double R = 6371000;

    double lat1 = arr[4][0];
    double lon1 = arr[4][1];
    double alt1 = arr[4][2];
    long long timestamp1 = arr[4][3];

    if (lat == 0) {
        lat = lat1;
        lon = lon1;
        alt = alt1;
        timestamp = timestamp1;
    } else {
        double lat2 = lat;
        double lon2 = lon;
        double alt2 = alt;
        long long timestamp2 = timestamp;

        lat = lat1;
        lon = lon1;
        alt = alt1;
        timestamp = timestamp1;

        double dlat = (lat2 - lat1) * M_PI / 180.0;
        double dlon = (lon2 - lon1) * M_PI / 180.0;
        double dalt = alt2 - alt1;

        double delta_y = R * dlon;
        double delta_x = R * dlat;
        double dt = (timestamp2 - timestamp1) / 1000.0;

        double vel_x = (dt != 0) ? delta_x/dt : 0;
        double vel_y = (dt != 0) ? delta_y/dt : 0;
        double vel_z = (dt != 0) ? dalt/dt : 0;

        std::vector<double> payload = {
            arr[0][0], arr[0][1], arr[0][2],  // accelerometer
            arr[1][0], arr[1][1], arr[1][2],  // gyroscope
            arr[2][0], arr[2][1], arr[2][2],  // magnetic field
            arr[4][0], arr[4][1], arr[4][2], static_cast<double>(arr[4][3]),  // GPS
            vel_x, vel_y, vel_z,
            arr[3][0]  // pressure
        };

        payloadHandler(payload);

        // std::cout << "payload = [";
        // for (size_t i = 0; i < payload.size(); ++i) {
        //     std::cout << payload[i];
        //     if (i < payload.size() - 1) std::cout << ", ";
        // }
        // std::cout << "]" << std::endl;
    }
}

void on_IMU(const json& values) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    if (ctrl == 0) {
        if (gps_sensor) {
            gps_sensor->send_requests("getLastKnownLocation");
        }
    }

    ctrl++;

    auto data = values["values"];
    auto type = values["type"].get<std::string>();
    
    auto it = std::find(types_order.begin(), types_order.end(), type);
    if (it != types_order.end()) {
        size_t index = std::distance(types_order.begin(), it);
        collected_values[index] = data;
    }

    bool all_collected = true;
    for (const auto& value : collected_values) {
        if (value.is_null()) {
            all_collected = false;
            break;
        }
    }

    if (all_collected) {
        calc_vel(collected_values);
        ctrl = 0;
        collected_values = std::vector<json>(types_order.size());
    }
}

void on_GPS(const json& values) {
    if (values.contains("lastKnowLocation") && values["lastKnowLocation"]) {
        std::vector<double> ar = {
            values["latitude"],
            values["longitude"],
            values["altitude"],
            values["time"]
        };
        auto it = std::find(types_order.begin(), types_order.end(), "gps");
        if (it != types_order.end()) {
            size_t index = std::distance(types_order.begin(), it);
            collected_values[index] = ar;
        }
    }
}

// Sensor class implementation
Sensor::Sensor(const std::string& address, const std::string& sensor_type,
               std::function<void(const json&)> callback)
    : address_(address), sensor_type_(sensor_type), callback_(callback) {
    
    endpoint_.clear_access_channels(websocketpp::log::alevel::all);
    endpoint_.init_asio();

    endpoint_.set_message_handler(bind(&Sensor::on_message, this, ::_1, ::_2));
    endpoint_.set_open_handler(bind(&Sensor::on_open, this, ::_1));
    endpoint_.set_fail_handler(bind(&Sensor::on_fail, this, ::_1));
    endpoint_.set_close_handler(bind(&Sensor::on_close, this, ::_1));
}

void Sensor::connect() {
    std::thread([this]() {
        try {
            websocketpp::lib::error_code ec;
            std::string uri = "ws://" + address_ + sensor_type_;
            connection_ = endpoint_.get_connection(uri, ec);
            
            if (ec) {
                std::cout << "Connect initialization error: " << ec.message() << std::endl;
                return;
            }

            endpoint_.connect(connection_);
            endpoint_.run();
        } catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    }).detach();
}

void Sensor::send_requests(const std::string& message) {
    try {
        endpoint_.send(connection_->get_handle(), message, websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        std::cout << "Send error: " << e.what() << std::endl;
    }
}

void Sensor::on_message(websocketpp::connection_hdl, websocket_client::message_ptr msg) {
    try {
        json j = json::parse(msg->get_payload());
        callback_(j);
    } catch (const std::exception& e) {
        std::cout << "Message parsing error: " << e.what() << std::endl;
    }
}

void Sensor::on_open(websocketpp::connection_hdl) {
    std::cout << "Connected to: " << address_ << std::endl;
}

void Sensor::on_fail(websocketpp::connection_hdl) {
    std::cout << "Connection failed" << std::endl;
}

void Sensor::on_close(websocketpp::connection_hdl) {
    std::cout << "Connection closed" << std::endl;
}

int main() {
    std::string address = "192.168.1.4:8080";

    imu_sensor = std::make_unique<Sensor>(
        address,
        "/sensors/connect?types=[\"android.sensor.accelerometer\",\"android.sensor.gyroscope\",\"android.sensor.magnetic_field\",\"android.sensor.pressure\"]",
        on_IMU
    );

    gps_sensor = std::make_unique<Sensor>(
        address,
        "/gps",
        on_GPS
    );

    imu_sensor->connect();
    gps_sensor->connect();

    // Que horror!
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}