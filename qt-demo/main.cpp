#include <QApplication>
#include "mqttclientwgt.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MqttClientWgt window;
    window.show();
    return app.exec();
}
