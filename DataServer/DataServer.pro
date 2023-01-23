#-------------------------------------------------
#
# Project created by QtCreator 2022-07-01
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += widgets
QT       += network
QT       += sql
QT       += concurrent

# Title
#
TARGET = DataServer
TEMPLATE = app

#Application icon
#
win32:RC_ICONS += icons/DataServer.ico


# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}


SOURCES += \
    ClientSocket.cpp \
    DataDevice.cpp \
    Database.cpp \
    DeviceBase.cpp \
    DeviceListDialog.cpp \
    DeviceSettingDialog.cpp \
    Options.cpp \
    OptionsDialog.cpp \
    PacketBase.cpp \
    PacketView.cpp \
    PanelDeviceList.cpp \
    PanelPictureView.cpp \
    ProcessData.cpp \
    ServerSocket.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    ClientSocket.h \
    DataDevice.h \
    Database.h \
    DeviceBase.h \
    DeviceListDialog.h \
    DevicePacket.h \
    DeviceSettingDialog.h \
    MainWindow.h \
    Options.h \
    OptionsDialog.h \
    PacketBase.h \
    PacketView.h \
    PanelDeviceList.h \
    PanelPictureView.h \
    ProcessData.h \
    ServerSocket.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#TRANSLATIONS = languages/DataServer_ru.ts

RESOURCES += \
    Resources.qrc

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

DISTFILES +=

LIBS += -lws2_32
