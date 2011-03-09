TARGET = stubgen
INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build
UI_DIR = build
QT = core

include(cplusplus/cplusplus-lib.pri)
SOURCES += main.cpp \
    StubGenerator.cpp \
    ImplFinder.cpp

HEADERS += \
    StubGenerator.h \
    ImplFinder.h
