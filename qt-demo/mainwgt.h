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
    void on_m_btnConnect_clicked();
    void on_m_btnDisconnect_clicked();
    void on_m_btnSubscribe_clicked();
    void on_m_btnPublish_clicked();
    void on_m_btnClearLog_clicked();

private:
    void appendMessage(const QString& msg, bool isSent);
    void updateConnectionState(bool connected);

    Ui::MainWgt* m_ui;
    QMQTT::Client* m_client;
};

#endif
