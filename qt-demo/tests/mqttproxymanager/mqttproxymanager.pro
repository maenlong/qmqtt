QT += core network testlib
CONFIG += c++11 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mqttproxymanager

INCLUDEPATH += $$PWD/../..

SOURCES += tst_mqttproxymanager.cpp \
           $$PWD/../../mqttproxymanager.cpp

HEADERS += $$PWD/../../mqttproxymanager.h
