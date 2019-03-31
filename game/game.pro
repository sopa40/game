TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -LLibSDL -lSDL2 -ldl -lpthread
INCLUDEPATH += LibSDL/include


SOURCES += main.c

