#-------------------------------------------------
#
# Project created by QtCreator 2011-10-15T22:10:46
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = interactiveKnightsTour
TEMPLATE = app

CONFIG += debug
CONFIG += console
CONFIG += static

SOURCES += main.cpp\
        mainwindow.cpp \
    GLWidget.cpp \
    light.cpp \
    model.cpp \
    particle.cpp \
    emitter.cpp \
    texture.cpp

HEADERS  += mainwindow.h \
    GLWidget.h \
    vector3.h \
    GLWidget.h \
    GLWidget.h \
    GLWidget.h \
    light.h \
    model.h \
    particle.h \
    emitter.h \
    texture.h

FORMS    += mainwindow.ui



win32: LIBS += -lOpenGL32

win32: LIBS += -lglu32
