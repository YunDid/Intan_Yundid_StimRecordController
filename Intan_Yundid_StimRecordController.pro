TEMPLATE      = app

QT            += widgets multimedia

CONFIG        += static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DeviceManager.cpp \
    main.cpp \
    okFrontPanelDLL.cpp \
    rhs2000datablock.cpp \
    rhs2000evalboard.cpp \
    rhs2000registers.cpp \
    startupdialog.cpp

HEADERS += \
    DeviceManager.h \
    globalconstants.h \
    okFrontPanelDLL.h \
    rhs2000datablock.h \
    rhs2000evalboard.h \
    rhs2000registers.h \
    startupdialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/ucrt"

LIBS += -L"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/ucrt/x64"
