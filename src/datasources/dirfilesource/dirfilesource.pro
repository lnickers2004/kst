include($$PWD/../../../kst.pri)

QT += xml qt3support

TEMPLATE = lib
CONFIG += plugin
OBJECTS_DIR = tmp
MOC_DIR = tmp
TARGET = kstdata_dirfilesource
DESTDIR = $$OUTPUT_DIR/plugin

INCLUDEPATH += \
    tmp \
    $$TOPLEVELDIR/src/libkst \
    $$OUTPUT_DIR/src/datasources/dirfilesource/tmp

LIBS += -lgetdata++ -lkst

SOURCES += \
    dirfilesource.cpp 

HEADERS += \
    dirfilesource.h 
