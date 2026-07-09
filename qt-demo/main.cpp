#include <QApplication>
#include "mqttclientwgt.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MqttClientWgt w;
    w.show();
    return a.exec();
}
