#ifndef MAINWGT_H
#define MAINWGT_H

#include <QWidget>
#include "qmqtt.h"

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
    void sig_connect();
    void sig_disconnect();
    void sig_subscribe();
    void sig_publish();

private:
    void appendMessage(const QString& msg, bool isSent);
    void updateConnectionState(bool connected);

    Ui::MainWgt* m_ui;
    QMQTT::Client* m_client;
};

#endif
