# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: ./welfenlab_comp06
# Das Target ist eine Anwendung:  welfenlab_comp06

RESOURCES = gui/data/gui.qrc translations.qrc
QT = core gui network widgets
MOC_DIR = moc 
UI_DIR = gui 
OBJECTS_DIR = obj
INCLUDEPATH += .
QMAKE_CXXFLAGS_DEBUG += -Wall \
                        -ansi 
CONFIG += release \
          warn_on \
          qt \
          thread \
          stl
TEMPLATE = app 
FORMS += gui/mainwindow.ui \
         gui/newgamedialog.ui \
         gui/aisettingsdialog.ui \
         gui/infodialog.ui \
         gui/licensedialog.ui \
         gui/chatwidget.ui \
         gui/newnetgamedialog.ui 
HEADERS += gui/mainwindow.h \
           backgammon.h \
           gui/dicewidget.h \
           gui/backgammonwidget.h \
           gui/flowlayout.h \
           config.h \
           gui/newgamedialog.h \
           ai.h \
           ai_std_config.h \
           aithread.h \
           gui/aisettingsdialog.h \
           netbackgammon.h \
           gui/chatwidget.h \
           gui/settings.h
SOURCES += main.cpp \
           gui/mainwindow.cpp \
           backgammon.cpp \
           gui/dicewidget.cpp \
           gui/backgammonwidget.cpp \
           gui/flowlayout.cpp \
           gui/newgamedialog.cpp \
           ai.cpp \
           aithread.cpp \
           gui/aisettingsdialog.cpp \
           netbackgammon.cpp \
           gui/chatwidget.cpp \
           gui/settings.cpp

TARGET = backgammon

TRANSLATIONS += backgammon_en.ts
ICON = "resources/MacOS/backgammon.icns"
RC_FILE = backgammon.rc
