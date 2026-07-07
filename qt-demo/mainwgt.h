#ifndef MAINWGT_H
#define MAINWGT_H

#include <QWidget>

namespace QMQTT {
class Client;
}

namespace Ui {
class MainWgt;
}

// MQTT 客户端演示主窗口
class MainWgt : public QWidget
{
    Q_OBJECT

public:
    explicit MainWgt(QWidget* parent = nullptr);
    ~MainWgt();

private slots:
    // UI 自动连接槽函数
    void on_connectBtn_clicked();       // 连接/重连 MQTT 代理
    void on_disconnectBtn_clicked();    // 断开连接
    void on_subscribeBtn_clicked();     // 手动订阅
    void on_publishBtn_clicked();       // 发布消息
    void on_clearLogBtn_clicked();      // 清空日志

private:
    void appendMessage(const QString& msg, bool isSent);          // 追加日志（>> 发送 / << 接收）
    void updateConnectionState(bool connected);                   // 更新连接状态显示

    Ui::MainWgt* m_ui;                   // UI 实例
    QMQTT::Client* m_client = nullptr;   // MQTT 连接实例
};

#endif
