#pragma once
#include <optional>
#include <string>
#include <queue>

#include "IPStack.h"
#include "Countdown.h"
#include "MQTTClient.h"

#include "garage_types.h"
#include "command_parser.h"

namespace garage {

class IMqttClient {
public:
    virtual ~IMqttClient() = default;

    virtual bool init() = 0;
    virtual bool connect() = 0;
    virtual void loop() = 0;
    virtual std::optional<RemoteCommand> poll_command() = 0;
    virtual bool publish_status(const std::string& payload) = 0;
    virtual bool publish_response(const std::string& payload) = 0;
};

class MqttClient : public IMqttClient {
public:
    struct Config {
        const char* wifi_ssid;
        const char* wifi_password;
        const char* broker_ip;
        int         broker_port      {1883};
        const char* client_id        {"pico_garage"};
        int         wifi_timeout_ms  {30000};   // 保留，不由本类直接使用
        int         yield_timeout_ms {100};
    };

    explicit MqttClient(const Config& cfg);
    ~MqttClient() override = default;

    bool init() override;
    bool connect() override;
    void loop() override;

    std::optional<RemoteCommand> poll_command() override;
    bool publish_status(const std::string& payload) override;
    bool publish_response(const std::string& payload) override;

    bool is_connected() const { return connected_; }

private:
    bool connect_broker();
    bool subscribe_command_topic();
    bool mqtt_publish(const char* topic, const std::string& payload);

    static void on_message(MQTT::MessageData& md);

    Config cfg_;
    bool   connected_ {false};

    IPStack                                  ip_stack_;
    MQTT::Client<IPStack, Countdown, 512, 2> client_;

    static std::queue<std::string> s_raw_cmd_queue_;

    static constexpr const char* kTopicStatus   = "garage/door/status";
    static constexpr const char* kTopicCommand  = "garage/door/command";
    static constexpr const char* kTopicResponse = "garage/door/response";
};

} // namespace garage