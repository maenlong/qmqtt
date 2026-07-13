QT += core testlib
CONFIG += c++11 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mqttreconnectpolicy

INCLUDEPATH += $$PWD/../..

SOURCES += tst_mqttreconnectpolicy.cpp \
           $$PWD/../../mqttreconnectpolicy.cpp

HEADERS += $$PWD/../../mqttreconnectpolicy.h
