#ifndef MQTTCLIENTMGR_H
#define MQTTCLIENTMGR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>

class QSslError;

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
    void subscribe(const QString& topic, quint8 qos);          // 订阅主题
    void unsubscribe(const QString& topic);                    // 取消订阅
    void publish(const QString& topic, const QByteArray& payload, quint8 qos); // 发布消息
    bool isConnected() const;                                  // 当前是否已连接

private:
    bool isConnectionParamsValid(const MqttConnectionParams& params) const; // 校验连接参数
    QString webSocketUrl(const MqttConnectionParams& params) const; // 构建 WebSocket URL
    void createClient(const MqttConnectionParams& params);      // 创建 qmqtt 客户端实例
    void applyClientConfig(const MqttConnectionParams& params); // 应用连接参数
    void forwardClientSignals();                                // 透传 qmqtt 信号

signals:
    void sig_connected();              // MQTT 连接成功
    void sig_disconnected();           // MQTT 断开连接
    void sig_error(int errorCode);     // MQTT 错误
    void sig_sslErrors(const QList<QSslError>& errors);  // SSL 证书错误
    void sig_messageReceived(const QString& topic, const QByteArray& payload);  // 收到消息
    void sig_subscribed(const QString& topic, quint8 qos); // 收到订阅确认（SUBACK）
    void sig_unsubscribed(const QString& topic); // 收到取消订阅确认（UNSUBACK）
    void sig_published(const QString& topic, quint8 qos, quint16 messageId); // 收到发布结果
    void sig_pingResp();               // 收到心跳回复（PINGRESP）

private:
    QMQTT::Client* m_client = nullptr;   // qmqtt 客户端实例
};

#endif
