TARGET = tzst
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
QT = core

DESTDIR = $$PWD

LIBS += -lzstd

SOURCES += \
        joinpath.cpp \
        main.cpp \
        zs.cpp

HEADERS += \
    joinpath.h \
    strformat.h \
    zs.h
