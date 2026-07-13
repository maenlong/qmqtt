#include "mqttclientwgt.h"

#include <QApplication>
#include <QString>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(QString("QLabel[mqttConnectionState=\"connected\"] { color: green; }\n"
                              "QLabel[mqttConnectionState=\"disconnected\"] { color: red; }"));
    MqttClientWgt window;
    window.show();
    return app.exec();
}
