QT += core gui widgets
CONFIG += c++14
DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lldap -llber -lresolv -lgsasl

SOURCES = src/*.cpp
HEADERS = src/*.h

SOURCES += src/active_directory.c

DESTDIR = .
OBJECTS_DIR = obj
MOC_DIR = moc
