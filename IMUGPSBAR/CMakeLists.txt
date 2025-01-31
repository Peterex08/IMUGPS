cmake_minimum_required(VERSION 3.14)
project(SensorMonitoring VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED system)

# Add main executable
add_executable(sensor_monitoring 
    src/main.cpp
    src/payload_handler.cpp
    src/uNavINS.cpp
)

include(FetchContent)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Add nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add WebSocket++
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.8.2
)
FetchContent_MakeAvailable(websocketpp)

# Configuração manual do Paho MQTT
set(PAHO_CPP_LIB_DIR "/usr/local/lib")
set(PAHO_CPP_INC_DIR "/usr/local/include")

# Set include directories
target_include_directories(sensor_monitoring
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${websocketpp_SOURCE_DIR}
        ${PAHO_CPP_INC_DIR}
)

# Link libraries
target_link_libraries(sensor_monitoring
    PRIVATE
        nlohmann_json::nlohmann_json
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        Boost::system
        ${PAHO_CPP_LIB_DIR}/libpaho-mqttpp3.so
        ${PAHO_CPP_LIB_DIR}/libpaho-mqtt3as.so
        pthread
)

# Set compiler flags
if(MSVC)
    target_compile_options(sensor_monitoring PRIVATE /W4)
else()
    target_compile_options(sensor_monitoring PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create project structure
set(PROJECT_STRUCTURE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

foreach(dir ${PROJECT_STRUCTURE})
    if(NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
    endif()
endforeach()