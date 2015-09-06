#-------------------------------------------------
#
# Project created by QtCreator 2015-08-17T15:24:28
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testfullscreen
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Protocol.c \
    serverthread.cpp

HEADERS  += mainwindow.h \
    Device.h \
    Protocol.h \
    plc_instruction.h \
    serverthread.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

DISTFILES +=

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android


