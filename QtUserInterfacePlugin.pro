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
    uirenderer.cpp \
    renderdispatcher.cpp

HEADERS += \
        qtuserinterfaceplugin.h \
    uirenderer.h \
    renderdispatcher.h

#target.path = "C:/Users/nezticle/Documents/Qt in VR/Assets/Plugins"
target.path = "D:/Code/Qt-VR-Unity/Assets/Plugins/QtVR"

INSTALLS += target

RESOURCES += \
    qml.qrc
