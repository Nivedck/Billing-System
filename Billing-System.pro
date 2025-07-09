QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Include paths for header files
INCLUDEPATH += src \
               src/mainwindow \
               src/adminwindow

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow/mainwindow.cpp \
    src/adminwindow/adminwindow.cpp

# Header files
HEADERS += \
    src/mainwindow/mainwindow.h \
    src/adminwindow/adminwindow.h

# UI forms
FORMS += \
    src/mainwindow/mainwindow.ui \
    src/adminwindow/adminwindow.ui

# Qt resources
RESOURCES += \
    resources/resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
