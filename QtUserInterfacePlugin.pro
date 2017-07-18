#-------------------------------------------------
#
# Project created by QtCreator 2017-07-14T14:09:58
#
#-------------------------------------------------

QT       += quick quick-private
CONFIG += dll

TARGET = QtUserInterfacePlugin
TEMPLATE = lib

INCLUDEPATH += $$PWD/Unity

SOURCES += \
        qtuserinterfaceplugin.cpp \
    uirenderer.cpp

HEADERS += \
        qtuserinterfaceplugin.h \
    uirenderer.h

target.path = "C:/Users/nezticle/Documents/Qt in VR/Assets/Plugins"

INSTALLS += target

RESOURCES += \
    qml.qrc
