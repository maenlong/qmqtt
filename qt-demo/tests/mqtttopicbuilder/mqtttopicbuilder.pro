QT += core testlib
CONFIG += c++11 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mqtttopicbuilder

INCLUDEPATH += $$PWD/../..

SOURCES += tst_mqtttopicbuilder.cpp \
           $$PWD/../../mqtttopicbuilder.cpp

HEADERS += $$PWD/../../mqtttopicbuilder.h
