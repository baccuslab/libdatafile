######################################################################
# Automatically generated by qmake (3.0) Mon Jul 20 21:04:21 2015
######################################################################

win32 {
	TEMPLATE = windows
}
unix {
	TEMPLATE = app
}
TARGET = daqsrv
VERSION = 0.2.0

INCLUDEPATH += . include
OBJECTS_DIR = build
MOC_DIR = build

QT += network widgets
CONFIG += c++11 debug_and_release
QMAKE_CXXFLAGS += -std=c++11


# Input
HEADERS += include/daqsrv.h \
           include/main.h \
           include/nidaq.h \
           include/serverwindow.h

SOURCES += src/daqsrv.cc \
           src/main.cc \
           src/nidaq.cc \
           src/serverwindow.cc
