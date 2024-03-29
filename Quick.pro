#-------------------------------------------------
#
# Project created by QtCreator 2019-10-11T12:21:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Quick
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

CONFIG += c++11

SOURCES += \
        arnoldnodedatamodel.cpp \
        arnoldriver.cpp \
        editclearwidget.cpp \
        filter.cpp \
        main.cpp \
        mainwindow.cpp \
        nodeentrymodel.cpp \
        pixelbuffer.cpp \
        quickparamwidget.cpp \
        quickscenewidget.cpp \
        renderview.cpp

HEADERS += \
        arnoldnodedatamodel.h \
        arnoldriver.h \
        color_fl.h \
        editclearwidget.h \
        filter.h \
        mainwindow.h \
        nodeentrymodel.h \
        pixelbuffer.h \
        quickparamwidget.h \
        quickscenewidget.h \
        renderview.h

FORMS += \
        editclearwidget.ui \
        mainwindow.ui \
        quickparamwidget.ui \
        quickscenewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../repo/ActionHero/Arnold/lib/ -lai \
                                      -Lc:/_/arnold/build-nodeeditor-Good-Release/lib/ -lnodes \


INCLUDEPATH += $$PWD/../ActionHero/Arnold/include \
            c:/_/arnold/nodeeditor/include
DEPENDPATH += $$PWD/../repo/ActionHero/Arnold/include \
            c:/_/arnold/nodeeditor/include
