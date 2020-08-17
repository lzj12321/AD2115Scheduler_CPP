#-------------------------------------------------
#
# Project created by QtCreator 2019-05-24T16:13:52
#
#-------------------------------------------------

QT       += core gui
QT+=network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SchedulerGUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lwiringPi

SOURCES += \
        main.cpp \
        schedulergui.cpp \
    io_control.cpp \
    gpio_pi.cpp \
    socket.cpp \
    timer.cpp

HEADERS += \
        schedulergui.h \
    io_control.h \
    gpio_pi.h \
    socket.h \
    scheduler.h \
    timer.h

FORMS += \
        schedulergui.ui \
    io_control.ui
