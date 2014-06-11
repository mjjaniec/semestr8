TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    src/commons.c \
    src/matrix.c \
    src/lab5.c

LIBS += -L/usr/lib/openmpi/ \
    -lm

QMAKE_CC = mpicc -std=c99

HEADERS += \
    src/commons.h \
    src/matrix.h
