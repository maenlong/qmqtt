#include <QApplication>
#include "mainwgt.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWgt w;
    w.show();
    return a.exec();
}
