#-------------------------------------------------
#
# Project created by QtCreator 2011-06-10T07:36:51
#
#-------------------------------------------------

QT       += core gui

TARGET = BioDisplay
TEMPLATE = app


SOURCES += main.cpp\
        BioDisplay.cpp \
    FaceTracker.cpp

HEADERS  += BioDisplay.h \
    FaceTracker.h

OTHER_FILES += \
    Readme.md

INCLUDEPATH += /Users/artm/SDK/VeriLook_4_0_Standard_SDK/include/MacOSX
LIBS += -L/Library/Frameworks/Neurotechnology
LIBS += -lNExtractors -lNMatchers -lNTemplates -lNCore -lNImages -lNLicensing






