TEMPLATE = app
TARGET = mqtt-client
QT += core widgets network websockets
CONFIG += c++11 QMQTT_WEBSOCKETS

include($$PWD/../qmqtt.pri)

FORMS += mainwgt.ui
SOURCES += main.cpp mainwgt.cpp
HEADERS += mainwgt.h
