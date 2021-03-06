RC_FILE = czmilApplyAudit.rc
RESOURCES = icons.qrc
contains(QT_CONFIG, opengl): QT += opengl
QT += 
INCLUDEPATH += /c/PFM_ABEv7.0.0_Win64/include
LIBS += -L/c/PFM_ABEv7.0.0_Win64/lib -lCZMIL -lnvutility
DEFINES += WIN32 NVWIN3X UINT32_C INT32_C
CONFIG += console
QMAKE_LFLAGS += 
######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 22 14:04:17 2020
######################################################################

TEMPLATE = app
TARGET = czmilApplyAudit
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += czmilApplyAudit.hpp \
           czmilApplyAuditDef.hpp \
           fileInputPage.hpp \
           fileInputPageHelp.hpp \
           optionPage.hpp \
           optionPageHelp.hpp \
           runPage.hpp \
           startPage.hpp \
           version.hpp
SOURCES += czmilApplyAudit.cpp \
           fileInputPage.cpp \
           main.cpp \
           optionPage.cpp \
           runPage.cpp \
           startPage.cpp
RESOURCES += icons.qrc
