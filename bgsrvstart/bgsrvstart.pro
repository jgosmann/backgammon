# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: ./welfenlab_comp06
# Das Target ist eine Anwendung:  welfenlab_comp06

RESOURCES = data/icons.qrc translations.qrc
QT = core gui xml network widgets
MOC_DIR = moc
OBJECTS_DIR = obj
INCLUDEPATH += ../bgserver ../backgammon .
QMAKE_CXXFLAGS_DEBUG += -Wall \
                        -ansi 
CONFIG += release \
          warn_on \
          qt \
          thread \
          stl
TEMPLATE = app 
FORMS += serveroptions.ui srvrunwin.ui
HEADERS += serveroptions.h \
           ../bgserver/serversettings.h \
           ../bgserver/bgserver.h \
           ../bgserver/bggame.h \
           ../bgserver/bgconnection.h \
           ../backgammon/config.h \
           ../backgammon/backgammon.h
SOURCES += main.cpp \
           serveroptions.cpp \
           ../bgserver/serversettings.cpp \
           ../bgserver/bgserver.cpp \
           ../bgserver/bggame.cpp \
           ../bgserver/bgconnection.cpp \
           ../backgammon/backgammon.cpp

TARGET = bgsrvstart

TRANSLATIONS += bgsrvstart_en.ts
ICON = "resources/MacOS/bgsrvstart.icns"
RC_FILE = bgsrvstart.rc
