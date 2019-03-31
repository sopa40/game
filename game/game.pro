TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L/home/nazar/Downloads/SDL2-2.0.9 -lSDL2 -ldl -lpthread
INCLUDEPATH += //home//nazar//Downloads//SDL2-2.0.9//include


SOURCES += main.c

