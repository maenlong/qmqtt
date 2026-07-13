#ifndef MQTTRECONNECTPOLICY_H
#define MQTTRECONNECTPOLICY_H

// MQTT 指数退避重连策略
class MqttReconnectPolicy
{
public:
    MqttReconnectPolicy();

    int currentDelayMs() const; // 返回当前重连等待时间
    void advance();             // 推进到下一次重连等待时间
    void reset();               // 恢复初始重连等待时间

private:
    int m_currentDelayMs = -1; // 当前重连等待时间（毫秒）
};

#endif
