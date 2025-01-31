#include "pld_handler.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::vector<float>> sensor_data;

void process_payload(const std::string& payload) {
    Json::Value jsonData;
    Json::Reader reader;

    if (reader.parse(payload, jsonData)) {
        if(jsonData.isMember("type") && jsonData.isMember("values")){
            std::string sensor_type = jsonData["type"].asString();
            const Json::Value& values = jsonData["values"];

            if (values.isArray() && values.size() == 3) {
                std::vector<float> value_array;

                for (int i=0; i<3; ++i) {
                    value_array.push_back(values[i].asFloat());
                }

                sensor_data[sensor_type] = value_array;

                if (sensor_data.size() == 3) {
                    std::vector<float> condensed_array;

                    for (const auto& type : {"android.sensor.gyroscope", "android.sensor.magnetic_field", "android.sensor.accelerometer"}) {
                        if (sensor_data.count(type)) {
                            condensed_array.insert(condensed_array.end(), sensor_data[type].begin(), sensor_data[type].end());
                        }
                    }

                    std::cout << "Condensed array: ";
                    for (size_t i = 0; i <condensed_array.size(); ++i) {
                        std::cout << condensed_array[i] << " ";
                        if (i<condensed_array.size()-1) std::cout <<", "<<std::endl;
                    }

                    sensor_data.clear();
                }
            }
        }
    }
}