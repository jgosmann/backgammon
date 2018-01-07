TEMPLATE = app

CONFIG += release \
console
CONFIG -= app_bundle
TARGET = bgserver

QT -= gui

QT += network \
xml
INCLUDEPATH += ../backgammon .

OBJECTS_DIR = obj

MOC_DIR = moc

SOURCES += main.cpp \
serversettings.cpp \
bgserver.cpp \
 bgconnection.cpp \
 ../backgammon/backgammon.cpp \
 bggame.cpp
HEADERS += serversettings.h \
bgserver.h \
bgconnection.h \
 bggame.h \
 ../backgammon/backgammon.h
