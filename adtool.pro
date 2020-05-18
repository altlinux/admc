QT += core gui widgets
CONFIG += c++14
DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lldap -llber -lresolv -lgsasl

SOURCES = src/*.cpp
HEADERS = src/*.h

SOURCES += src/active_directory.c

DESTDIR = .
OBJECTS_DIR = obj
UI_DIR = src/ui
MOC_DIR = moc

FORMS += \
    data/ui/mainwindow.ui


