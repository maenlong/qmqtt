#ifndef MQTTCLIENTMGR_H
#define MQTTCLIENTMGR_H

#include "mqttreconnectpolicy.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>

class QSslError;
class QTimer;

namespace QMQTT {
class Client;
}

enum MqttConnectionType
{
    MqttConnectionTcp = 0,
    MqttConnectionWs,
    MqttConnectionWss
};

// MQTT 连接参数
struct MqttConnectionParams
{
    QString host = QString("");    // Broker 地址
    quint16 port = 1883;           // Broker 端口
    MqttConnectionType type = MqttConnectionTcp; // 连接类型
    QString clientId = QString(""); // 客户端标识
    QString username = QString(""); // 认证用户名
    QString password = QString(""); // 认证密码
    int     keepAlive = 30;        // 心跳间隔（秒）
    bool    cleanSession = true;   // 是否清理会话
    QString websocketPath = QString("/mqtt"); // WebSocket 路径
    QString websocketOrigin = QString(""); // WebSocket Origin
    QString sslCaCertPath = QString(""); // CA 证书文件路径
    bool    ignoreSelfSigned = false; // 是否忽略自签名证书错误
    QString willTopic = QString(""); // 遗嘱主题
    QString willMessage = QString(""); // 遗嘱消息
    int     willQos = 0;           // 遗嘱 QoS
    bool    willRetain = false;    // 是否保留遗嘱
};

// MQTT 客户端管理类，封装连接、订阅、发布逻辑，与 UI 无关
class MqttClientMgr : public QObject
{
    Q_OBJECT

public:
    explicit MqttClientMgr(QObject* parent = nullptr);
    ~MqttClientMgr();

    bool connectToHost(const MqttConnectionParams& params);    // 校验参数并连接 MQTT Broker
    void disconnectFromHost();                                 // 断开连接
    bool subscribe(const QString& topic, quint8 qos);          // 校验并发起订阅请求
    bool unsubscribe(const QString& topic);                    // 校验并发起取消订阅请求
    bool publish(const QString& topic, const QByteArray& payload, quint8 qos); // 校验并发起发布请求
    bool isConnected() const;                                  // 当前是否已连接

private:
    bool isConnectionParamsValid(const MqttConnectionParams& params) const; // 校验连接参数
    bool isPermanentConnectionError(int errorCode) const;          // 判断连接错误是否不可自动恢复
    QString webSocketUrl(const MqttConnectionParams& params) const; // 构建 WebSocket URL
    void createClient(const MqttConnectionParams& params);      // 创建 qmqtt 客户端实例
    void applyClientConfig(const MqttConnectionParams& params); // 应用连接参数
    void forwardClientSignals();                                // 透传 qmqtt 信号
    void resetReconnectState();                                 // 重置自动重连状态
    void scheduleReconnect();                                   // 按指数退避计划下次重连

signals:
    void sig_connected();              // MQTT 连接成功
    void sig_disconnected();           // MQTT 断开连接
    void sig_error(int errorCode);     // MQTT 错误
    void sig_connectionParamsInvalid(); // MQTT 连接参数无效
    void sig_caCertificateLoadFailed(const QString& path); // CA 证书加载失败
    void sig_sslErrors(const QList<QSslError>& errors);  // SSL 证书错误
    void sig_messageReceived(const QString& topic, const QByteArray& payload);  // 收到消息
    void sig_subscribed(const QString& topic, quint8 qos); // 收到订阅确认（SUBACK）
    void sig_unsubscribed(const QString& topic); // 收到取消订阅确认（UNSUBACK）
    void sig_published(const QString& topic, quint8 qos, quint16 messageId); // 收到发布结果
    void sig_pingResp();               // 收到心跳回复（PINGRESP）
    void sig_reconnectScheduled(int delaySeconds); // 已计划自动重连
    void sig_reconnectStopped(int errorCode);      // 永久错误导致自动重连停止

private slots:
    void slot_reconnect();             // 执行自动重连

private:
    QMQTT::Client* m_client = nullptr;   // qmqtt 客户端实例
    QTimer* m_reconnectTimer = nullptr;   // 自动重连定时器
    MqttReconnectPolicy m_reconnectPolicy; // 指数退避重连策略
    bool m_manualDisconnect = false;      // 是否由用户主动断开
    bool m_reconnectAllowed = false;      // 当前连接是否允许自动重连
};

#endif
