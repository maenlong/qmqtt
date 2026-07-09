#ifndef MQTTCLIENTWGT_H
#define MQTTCLIENTWGT_H

#include <QWidget>
#include <QList>

class QSslError;
class QTranslator;
class MqttClientMgr;

namespace Ui {
class MqttClientWgt;
}

// MQTT 客户端演示窗口
class MqttClientWgt : public QWidget
{
    Q_OBJECT

public:
    explicit MqttClientWgt(QWidget* parent = nullptr);
    ~MqttClientWgt();

private:
    void appendMessage(const QString& msg, bool isSent);          // 追加日志（>> 发送 / << 接收）
    void updateConnectionState(bool connected);                   // 更新连接状态显示
    void applyTranslations();                                     // 刷新界面翻译文案

protected:
    void changeEvent(QEvent* event) override;   // 动态语言切换时刷新界面文案

private slots:
    void on_connectBtn_clicked();       // 连接/重连 MQTT 代理
    void on_disconnectBtn_clicked();    // 断开连接
    void on_subscribeBtn_clicked();     // 手动订阅
    void on_unsubscribeBtn_clicked();   // 取消订阅
    void on_publishBtn_clicked();       // 发布消息
    void on_clearLogBtn_clicked();      // 清空日志
    void on_langCbx_currentIndexChanged(int index);   // 语言切换
    void on_browseCaCertBtn_clicked();                // 浏览 CA 证书文件

    void slot_onTypeChanged(int index);   // 连接类型切换时调整默认端口
    void slot_onConnected();              // MQTT 连接成功
    void slot_onDisconnected();           // MQTT 断开连接
    void slot_onError(int errorCode);     // MQTT 错误
    void slot_onSslErrors(const QList<QSslError>& errors);  // SSL 错误
    void slot_onMessageReceived(const QString& topic, const QByteArray& payload);  // 收到消息
    void slot_onPingResp();                               // Ping 响应确认
    void slot_onApplyProxy();                 // 应用代理设置

private:
    Ui::MqttClientWgt* m_ui = nullptr;        // UI 实例
    MqttClientMgr* m_mqttMgr = nullptr;       // MQTT 业务层
    QTranslator* m_translator = nullptr;      // 运行时翻译器
    int m_pingCount = 0;                      // Ping 计数器
};

#endif
