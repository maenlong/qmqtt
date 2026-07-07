#include <QApplication>
#include <QTranslator>
#include "mainwgt.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    if (translator.load("mqtt-client_zh_CN.qm", ":/i18n/"))
        a.installTranslator(&translator);
    MainWgt w;
    w.show();
    return a.exec();
}
