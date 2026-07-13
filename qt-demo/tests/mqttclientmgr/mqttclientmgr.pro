QT += core network websockets testlib
CONFIG += c++11 console testcase QMQTT_WEBSOCKETS
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mqttclientmgr

INCLUDEPATH += $$PWD/../..

include($$PWD/../../../qmqtt.pri)

SOURCES += tst_mqttclientmgr.cpp \
           $$PWD/../../mqttclientmgr.cpp

HEADERS += $$PWD/../../mqttclientmgr.h
