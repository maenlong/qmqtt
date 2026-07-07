TEMPLATE = app
TARGET = mqtt-client
QT += core widgets network websockets
CONFIG += c++11 QMQTT_WEBSOCKETS

include($$PWD/../qmqtt.pri)

FORMS += mainwgt.ui
SOURCES += main.cpp mainwgt.cpp
HEADERS += mainwgt.h

win32 {
    BIN_DIR = $$shell_path($$PWD/bin)
    QMAKE_POST_LINK += if exist $$BIN_DIR $(COPY_DIR) $$BIN_DIR $(DESTDIR)
}
