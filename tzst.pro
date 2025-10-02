DESTDIR = $$PWD/_bin
QMAKE_PROJECT_DEPTH=0
TARGET = tzst
TEMPLATE = app
CONFIG -= app_bundle
CONFIG += console c++17 nostrip debug_info
#QT = core

QMAKE_CXXFLAGS += -g

win32 {
	INCLUDEPATH += $$PWD/../zstd/lib
#	LIBS += -L$$PWD/../zstd\build\VS2010\bin\x64_Release -llibzstd
	LIBS += C:/develop/zstd/build/VS2010/bin/x64_Release/libzstd_static.lib
}

!win32 {
	LIBS += -lzstd
}

SOURCES += \
        base64.cpp \
        joinpath.cpp \
        main.cpp \
        misc.cpp \
        tar.cpp \
        tzst.cpp \
        zs.cpp

HEADERS += \
    base64.h \
    joinpath.h \
    misc.h \
    tar.h \
    tzst.h \
    zs.h
