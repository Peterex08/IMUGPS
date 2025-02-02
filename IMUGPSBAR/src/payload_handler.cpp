#include "uNavINS.h"
#include <iostream>
#include "../include/payload_handler.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <string>
#include <mqtt/async_client.h>

#define PI 3.14159265358979323846

class MqttPublisher {
private:
    std::string server_address;
    std::string client_id;
    std::string topic;
    mqtt::async_client client;
    mqtt::connect_options connOpts;
    
    const int MAX_RETRIES = 3;
    const std::chrono::seconds RETRY_INTERVAL{5};

public:
    MqttPublisher(const std::string& server, const std::string& id, const std::string& tpc)
        : server_address(server), 
          client_id(id), 
          topic(tpc), 
          client(server, id),
          connOpts(mqtt::connect_options_builder()
                .keep_alive_interval(std::chrono::seconds(20))
                .clean_session(true)
                .automatic_reconnect(true)  // Habilita reconexão automática
                .finalize()) {
        connect();
    }

    bool ensureConnected() {
        if (!client.is_connected()) {
            std::cout << "Conexão perdida. Tentando reconectar..." << std::endl;
            return connect();
        }
        return true;
    }

    bool connect() {
        for (int attempts = 0; attempts < MAX_RETRIES; ++attempts) {
            try {
                std::cout << "Tentativa de conexão " << (attempts + 1) << " de " << MAX_RETRIES << std::endl;
                client.connect(connOpts)->wait();
                std::cout << "Conectado com sucesso!" << std::endl;
                return true;
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "Erro ao conectar: " << exc.what() << std::endl;
                if (attempts < MAX_RETRIES - 1) {
                    std::cout << "Aguardando " << RETRY_INTERVAL.count() << " segundos antes de tentar novamente..." << std::endl;
                    std::this_thread::sleep_for(RETRY_INTERVAL);
                }
            }
        }
        return false;
    }

    bool publish(const std::string& payload) {
        try {
            if (!ensureConnected()) {
                std::cerr << "Não foi possível estabelecer conexão para publicar." << std::endl;
                return false;
            }

            mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload);
            pubmsg->set_qos(1);

            client.publish(pubmsg)->wait();
            return true;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Erro ao publicar: " << exc.what() << std::endl;
            return false;
        }
    }

    ~MqttPublisher() {
        try {
            if (client.is_connected()) {
                client.disconnect()->wait();
            }
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Erro ao desconectar: " << exc.what() << std::endl;
        }
    }

    bool publishBinary(const uint8_t* data, size_t length) {
        try {
            if (!ensureConnected()) {
                std::cerr << "Não foi possível estabelecer conexão para publicar." << std::endl;
                return false;
            }

            mqtt::message_ptr pubmsg = mqtt::make_message(topic, data, length);
            pubmsg->set_qos(1);

            client.publish(pubmsg)->wait();
            return true;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Erro ao publicar: " << exc.what() << std::endl;
            return false;
        }
    }
};

static std::unique_ptr<MqttPublisher> globalPublisher = nullptr;

#pragma pack(push, 1)  // Garante que não haja padding entre os campos
struct CompactPayload {
    float time;
    float lat;
    float lon;
    float alt;
    float veld;
    float veln;
    float vele;
    float yaw;
    float pitch;
    float roll;
    float bar;
    float parachute;
};
#pragma pack(pop)

MqttPublisher* getPublisher() {
    if (!globalPublisher) {
        std::string server_address = "tcp://broker.hivemq.com:1883";
        std::string client_id = "cpp_publisher";
        std::string topic = "epta_strean/payloadata223"; //era pra ser stream, mas com preguiça de rebuildar deixei strean
        
        globalPublisher = std::make_unique<MqttPublisher>(server_address, client_id, topic);
    }
    return globalPublisher.get();
}

enum EstadoFoguete {
    PARADO = 0,
    SUBIDA = 1,
    APOGEU = 2,
    QUEDA_LIVRE = 3,
    ACIONA_PARAQUEDAS = 4,
    DESCIDA_SEGURA = 5
};

class ControleParaquedas {
private:
    EstadoFoguete estado;
    double alturaAnterior;
    double tempoNoApogeu;
    double ultimoTimestamp;
    const double LIMITE_VELOCIDADE_DESCIDA = -2.0;
    const double ALTURA_ABERTURA_PARAQUEDAS = 1000;
    const double TEMPO_MIN_APOGEU = 0.3;
    const double PRESSAO_ABERTURA_PARAQUEDAS = 900;

public:
    ControleParaquedas() : estado(SUBIDA), alturaAnterior(0), tempoNoApogeu(0), ultimoTimestamp(0) {}

    int atualizar(double altura, double velocidadeVertical, double pressao, long timestampUnix) {
        double deltaTempo = 0;
        if (ultimoTimestamp != 0) {
            deltaTempo = (timestampUnix - ultimoTimestamp) / 1000.0;
        }
        ultimoTimestamp = timestampUnix;

        switch (estado) {
            case SUBIDA:
                if (velocidadeVertical <= 0) { 
                    estado = APOGEU;
                    tempoNoApogeu = 0;
                }
                break;

            case APOGEU:
                tempoNoApogeu += deltaTempo;
                if (velocidadeVertical < -0.5 && tempoNoApogeu > TEMPO_MIN_APOGEU) {
                    estado = QUEDA_LIVRE;
                }
                break;

            case QUEDA_LIVRE:
                if (velocidadeVertical < LIMITE_VELOCIDADE_DESCIDA && 
                    (altura <= ALTURA_ABERTURA_PARAQUEDAS || pressao >= PRESSAO_ABERTURA_PARAQUEDAS)) {
                    estado = ACIONA_PARAQUEDAS;
                }
                break;

            case ACIONA_PARAQUEDAS:
                estado = DESCIDA_SEGURA;
                break;

            case DESCIDA_SEGURA:
                break;
        }

        alturaAnterior = altura;
        return static_cast<int>(estado);
    }
};

void payloadHandler(const std::vector<double>& payload) {

    double accel_x = payload[0];
    double accel_y = payload[1];
    double accel_z = payload[2];

    // Process gyroscope data
    double gyro_x = payload[3];
    double gyro_y = payload[4];
    double gyro_z = payload[5];
 
    // Process magnetic field data
    double mag_x = payload[6];
    double mag_y = payload[7];
    double mag_z = payload[8];

    // Process GPS data
    double lat = payload[9];
    double lon = payload[10];
    double alt = payload[11];
    double time = payload[12];

    // Process velocity data
    double vel_x = payload[13];
    double vel_y = payload[14];
    double vel_z = payload[15];

    // Process pressure data
    double pressure = payload[16];

    auto publisher = getPublisher();
    
    uNavINS ins;
    ControleParaquedas controle;

    ins.update(time, vel_x, vel_y, vel_z, lat*PI/180.0f, lon*PI/180.0f, alt, gyro_y, -1*gyro_x, gyro_z, accel_y, -1*accel_x, accel_z, mag_x, mag_y, mag_z);
    // std::cout << "Estimated position: " << ins.getLatitude_rad() << ", " << ins.getLongitude_rad() << ", " << ins.getAltitude_m() << std::endl;
    // std::cout << "Attitude: " << ins.getRoll_rad()*180/PI << ", " << ins.getPitch_rad()*180/PI << ", " << ins.getHeading_rad()*180/PI << ", " << std::endl;

    int estado = controle.atualizar(ins.getAltitude_m(), ins.getVelDown_ms(), pressure, time);

    CompactPayload compactData{
        static_cast<float>(time),
        static_cast<float>(ins.getLatitude_rad()*180/PI),
        static_cast<float>(ins.getLongitude_rad()*180/PI),
        static_cast<float>(ins.getAltitude_m()),
        static_cast<float>(ins.getVelDown_ms()),
        static_cast<float>(ins.getVelNorth_ms()),
        static_cast<float>(ins.getVelEast_ms()),
        static_cast<float>(ins.getHeading_rad()*180/PI),
        static_cast<float>(ins.getPitch_rad()*180/PI),
        static_cast<float>(ins.getRoll_rad()*180/PI),
        static_cast<float>(pressure),
        static_cast<float>(estado)
    };

    publisher->publishBinary(reinterpret_cast<const uint8_t*>(&compactData), sizeof(CompactPayload));

};

