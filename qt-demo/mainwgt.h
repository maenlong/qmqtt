#ifndef MAINWGT_H
#define MAINWGT_H

#include <QWidget>

namespace QMQTT {
class Client;
}

namespace Ui {
class MainWgt;
}

class MainWgt : public QWidget
{
    Q_OBJECT

public:
    explicit MainWgt(QWidget* parent = nullptr);
    ~MainWgt();

private slots:
    void on_connectBtn_clicked();
    void on_disconnectBtn_clicked();
    void on_subscribeBtn_clicked();
    void on_publishBtn_clicked();
    void on_clearLogBtn_clicked();

private:
    void appendMessage(const QString& msg, bool isSent);
    void updateConnectionState(bool connected);

    Ui::MainWgt* m_ui;
    QMQTT::Client* m_client = nullptr;
};

#endif
