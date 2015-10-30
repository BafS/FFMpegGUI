#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T16:27:58
#
#-------------------------------------------------

QT       += core gui multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FFMpegGUI
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

CONFIG += c++11

RESOURCES += \
    ressources.qrc

DISTFILES += \
    css/main.css \
    main.css
