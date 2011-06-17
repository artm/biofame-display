QT       += core gui

TARGET = BioDisplay
TEMPLATE = app


SOURCES += main.cpp\
        BioDisplay.cpp \
    BiometricThread.cpp \
    Verilook.cpp \
    LogHub.cpp

HEADERS  += BioDisplay.h \
    BiometricThread.h \
    CommonTypes.h \
    Verilook.h \
    LogHub.h

OTHER_FILES += \
    Readme.md \
    deploy-display.sh

INCLUDEPATH += /Users/artm/src/SDK/VeriLook_4_0_Standard_SDK/include/MacOSX
LIBS += -L/Library/Frameworks/Neurotechnology
LIBS += -lNExtractors -lNMatchers -lNTemplates -lNCore -lNImages -lNLicensing




















