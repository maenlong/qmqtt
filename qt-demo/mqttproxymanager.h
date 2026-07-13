#ifndef MQTTPROXYMANAGER_H
#define MQTTPROXYMANAGER_H

#include <QString>

enum MqttProxyType
{
    MqttProxyNone = 0,
    MqttProxyHttp,
    MqttProxySocks5
};

// MQTT 代理配置参数
struct MqttProxyParams
{
    MqttProxyType type = MqttProxyNone; // 代理类型
    QString host = QString("");         // 代理主机名或 IP 地址
    quint16 port = 0;                    // 代理端口
    QString username = QString("");     // 代理认证用户名
    QString password = QString("");     // 代理认证密码
};

// MQTT 全局代理配置管理类
class MqttProxyManager
{
public:
    static bool applyProxy(const MqttProxyParams& params); // 校验并应用全局代理配置
};

#endif
