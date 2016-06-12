QT += core
QT -= gui

CONFIG += c++11

TARGET = Core-Yash
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tcpserver.cpp \
    tcpconnection.cpp \
    mutex.cpp \
    core_yash.cpp \
    main.cpp \
    tcpclient.cpp

DISTFILES += \
    README.md

HEADERS += \
    tcpserver.h \
    mutex.h \
    cyexception.h \
    core_yash.h \
    tcpclient.h
