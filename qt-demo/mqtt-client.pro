TEMPLATE = app
TARGET = mqtt-client
QT += core widgets network websockets
CONFIG += c++11 QMQTT_WEBSOCKETS

include($$PWD/../qmqtt.pri)

TRANSLATIONS = mqtt-client_zh_CN.ts

RESOURCES += mqtt-client.qrc
FORMS += mqttclientwgt.ui
SOURCES += main.cpp mqttclientwgt.cpp mqttclientmgr.cpp mqttreconnectpolicy.cpp \
           mqtttopicbuilder.cpp mqttproxymanager.cpp
HEADERS += mqttclientwgt.h mqttclientmgr.h mqttreconnectpolicy.h \
           mqtttopicbuilder.h mqttproxymanager.h

win32 {
    BIN_DIR = $$shell_path($$PWD/bin)
    QMAKE_POST_LINK += if exist $$BIN_DIR $(COPY_DIR) $$BIN_DIR $(DESTDIR)
}
