#ifndef MQTTCLIENTWGT_H
#define MQTTCLIENTWGT_H

#include <QWidget>

class QTranslator;

namespace QMQTT {
class Client;
}

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
    void on_publishBtn_clicked();       // 发布消息
    void on_clearLogBtn_clicked();      // 清空日志
    void on_langCbx_currentIndexChanged(int index);   // 语言切换

private:
    Ui::MqttClientWgt* m_ui = nullptr;        // UI 实例
    QMQTT::Client* m_client = nullptr;        // MQTT 连接实例
    QTranslator* m_translator = nullptr;      // 运行时翻译器
};

#endif
