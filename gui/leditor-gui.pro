QT += core widgets

CONFIG += c++17

TARGET = leditor-gui
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    customtextwidget.cpp \
    fileexplorer.cpp \
    editortabs.cpp

HEADERS += \
    mainwindow.h \
    customtextwidget.h \
    fileexplorer.h \
    editortabs.h

# macOS specific settings
macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
} 