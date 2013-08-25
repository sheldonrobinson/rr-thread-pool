#-------------------------------------------------
#
# Project created by QtCreator 2013-08-24T05:36:08
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = ThreadPool
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += MessageQueue.h \
    ThreadPool.h \
    ThreadPosix.h

SOURCES += main.cpp \
    MessageQueue.cpp \
    ThreadPool.cpp \
    test_MessageQueue.cpp \
    test_ThreadPool.cpp

macx {
    LIBS += -stdlib=libc++

    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7
    QMAKE_LFLAGS += -mmacosx-version-min=10.7
}
