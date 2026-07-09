#include <QApplication>
#include "mqttclientwgt.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MqttClientWgt w;
    w.show();
    return app.exec();
}
