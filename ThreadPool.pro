#-------------------------------------------------
#
# Project created by QtCreator 2013-08-24T05:36:08
#
#-------------------------------------------------

cache()

QT       -= core
QT       -= gui

TARGET = ThreadPool
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

HEADERS += \
    Cond.h \
    Message.h \
    MessageQueue.h \
    Mutex.h \
    Task.h \
    Thread.h \
    ThreadPool.h \
    Trace.h

SOURCES += \
    Cond.cpp \
    MessageQueue.cpp \
    Mutex.cpp \
    Thread.cpp \
    ThreadPool.cpp \
    Trace.cpp \
    test_MessageQueue.cpp \
    test_Thread.cpp \
    test_ThreadPool.cpp \
    main.cpp

# May be necessary for older versions of QTCreator:
# macx {
#    LIBS += -stdlib=libc++
#    QMAKE_CXXFLAGS += -stdlib=libc++
#    QMAKE_CXXFLAGS += -std=c++11
#    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7
#    QMAKE_LFLAGS += -stdlib=libc++
#    QMAKE_LFLAGS += -mmacosx-version-min=10.7
# }
