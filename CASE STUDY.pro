QT += core gui webenginewidgets webchannel
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SymSpellChecker
TEMPLATE = app

SOURCES += SymSpellChecker.cpp
RESOURCES += resources.qrc

CONFIG += c++11