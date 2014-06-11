TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    rows.c \
    columns.c

 QMAKE_CC= gcc -std=c99

