#include "mqtt_client.h"
#include "status_serializer.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/err.h"

#include <cstdio>
#include <cstring>

namespace garage {

std::queue<std::string> MqttClient::s_raw_cmd_queue_;

MqttClient::MqttClient(const Config& cfg)
    : cfg_(cfg),
      ip_stack_(cfg.wifi_ssid, cfg.wifi_password),
      client_(ip_stack_) {}

bool MqttClient::init() {
    printf("[MQTT] init: using teacher IPStack for WiFi/CYW43\n");
    return true;
}

bool MqttClient::connect() {
    if (!connect_broker() || !subscribe_command_topic()) {
        connected_ = false;
        ip_stack_.disconnect();
        return false;
    }
    return true;
}

void MqttClient::loop() {
    cyw43_arch_poll();

    if (connected_) {
        const int rc = client_.yield(cfg_.yield_timeout_ms);
        if (rc != 0) {
            printf("[MQTT] yield error %d - broker disconnected\n", rc);
            connected_ = false;
            ip_stack_.disconnect();
        }
    }
}

std::optional<RemoteCommand> MqttClient::poll_command() {
    if (s_raw_cmd_queue_.empty()) {
        return std::nullopt;
    }

    std::string raw = std::move(s_raw_cmd_queue_.front());
    s_raw_cmd_queue_.pop();

    auto maybe = CommandParser::parse(raw);
    if (!maybe) {
        CommandResponse err;
        err.success = false;
        err.command = raw;
        err.message = "Unknown command: " + raw;
        publish_response(StatusSerializer::to_response_json(err));
        printf("[MQTT] unknown command rejected: \"%s\"\n", raw.c_str());
        return std::nullopt;
    }

    printf("[MQTT] command received: \"%s\"\n", raw.c_str());
    return maybe;
}

bool MqttClient::publish_status(const std::string& payload) {
    return mqtt_publish(kTopicStatus, payload);
}

bool MqttClient::publish_response(const std::string& payload) {
    return mqtt_publish(kTopicResponse, payload);
}

bool MqttClient::connect_broker() {
    printf("[MQTT] connecting to broker %s:%d ...\n",
           cfg_.broker_ip, cfg_.broker_port);

    const int tcp_rc = ip_stack_.connect(cfg_.broker_ip, cfg_.broker_port);
    if (tcp_rc != ERR_OK) {
        printf("[MQTT] TCP connect start failed (rc=%d)\n", tcp_rc);
        return false;
    }

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = const_cast<char*>(cfg_.client_id);
    data.keepAliveInterval = 60;
    data.cleansession = 1;

    const int mqtt_rc = client_.connect(data);
    if (mqtt_rc != 0) {
        printf("[MQTT] MQTT connect failed (rc=%d)\n", mqtt_rc);
        return false;
    }

    connected_ = true;
    printf("[MQTT] connected as \"%s\"\n", cfg_.client_id);
    return true;
}

bool MqttClient::subscribe_command_topic() {
    const int rc = client_.subscribe(kTopicCommand, MQTT::QOS0, on_message);
    if (rc != 0) {
        printf("[MQTT] subscribe to \"%s\" failed (rc=%d)\n",
               kTopicCommand, rc);
        return false;
    }

    printf("[MQTT] subscribed to \"%s\"\n", kTopicCommand);
    return true;
}

bool MqttClient::mqtt_publish(const char* topic, const std::string& payload) {
    if (!connected_) {
        return false;
    }

    MQTT::Message msg;
    msg.qos = MQTT::QOS0;
    msg.retained = false;
    msg.dup = false;
    msg.payload = const_cast<char*>(payload.c_str());
    msg.payloadlen = static_cast<int>(payload.size());

    const int rc = client_.publish(topic, msg);
    if (rc != 0) {
        printf("[MQTT] publish to \"%s\" failed (rc=%d)\n", topic, rc);
        connected_ = false;
        ip_stack_.disconnect();
        return false;
    }

    return true;
}

void MqttClient::on_message(MQTT::MessageData& md) {
    const size_t len = md.message.payloadlen;
    std::string raw(static_cast<const char*>(md.message.payload), len);
    s_raw_cmd_queue_.push(std::move(raw));
}

} // namespace garage