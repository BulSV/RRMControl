QT       += widgets serialport

TEMPLATE = app
win32:RC_FILE = RRMControl.rc
#CONFIG += console
CONFIG(debug, debug|release) {
    unix:DESTDIR = ../debug/unix
    unix:OBJECTS_DIR = ../debug/unix/objects
    unix:MOC_DIR = ../debug/unix/moc
    unix:RCC_DIR = ../debug/unix/moc

    win32:DESTDIR = ../debug/win32
    win32:OBJECTS_DIR = ../debug/win32/objects
    win32:MOC_DIR = ../debug/win32/moc
    win32:RCC_DIR = ../debug/win32/moc

    TARGET = RRMControlProd

    DEFINES += DEBUG
}
else {
    unix:DESTDIR = ../release/unix
    unix:OBJECTS_DIR = ../release/unix/objects
    unix:MOC_DIR = ../release/unix/moc
    unix:RCC_DIR = ../release/unix/moc

    win32:DESTDIR = ../release/win32
    win32:OBJECTS_DIR = ../release/win32/objects
    win32:MOC_DIR = ../release/win32/moc
    win32:RCC_DIR = ../release/win32/moc

    TARGET = RRMControlPro
}

HEADERS += \
    ComPort.h \
    Dialog.h \
    IProtocol.h \
    RRMProtocol.h

SOURCES += \
    ComPort.cpp \
    Dialog.cpp \
    main.cpp \
    RRMProtocol.cpp

RESOURCES += \
    RRMControl.qrc
