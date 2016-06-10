QT += core
QT -= gui

CONFIG += c++11

TARGET = Core-Yash
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.c \
    tcpserver.cpp \
    tcpconnection.cpp \
    mutex.cpp \
    core_yash.cpp

DISTFILES += \
    Core-Yash.pro.user

HEADERS += \
    tcpserver.h \
    mutex.h \
    cyexception.h \
    core_yash.h
