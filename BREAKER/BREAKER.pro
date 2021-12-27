QT -= gui
QT += serialport

LIBS += -lsetupapi -lhid -lwinusb -lstdc++fs

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET_CUSTOM_EXT = .exe
DEPLOY_COMMAND = windeployqt

DEPLOY_TARGET = C:/Users/Zhivoglyadov/Documents/Program_QT/LineBreaker/build-BREAKER-Desktop_Qt_5_14_2_MinGW_32_bit-Release/release

QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        diskpart.cpp \
        linebreaker.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    diskpart.h \
    linebreaker.h
