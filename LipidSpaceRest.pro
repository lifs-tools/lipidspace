QT       += core widgets

CONFIG += c++17 debug_and_release
QMAKE_CXXFLAGS += -fopenmp -march=native -Wno-unknown-pragmas


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RC_ICONS = LipidSpace.ico

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

!win32 {
    LIBS += -fopenmp -lcppGoslin -L /usr/lib/x86_64-linux-gnu -l openblas libraries/OpenXLSX/bin/linux/libOpenXLSX.a
    INCLUDEPATH += $$PWD/libraries/OpenXLSX/include
    DEPENDPATH += $$PWD/libraries/OpenXLSX/include
    SOURCES += \
        src/globaldata.cpp \
        src/Matrix.cpp \
        src/AssistanceFunctions.cpp \
        src/lipidspace.cpp \
        src/logging.cpp \
        src/lipidspacerest.cpp
    HEADERS += \
        lipidspace/globaldata.h \
        lipidspace/Matrix.h \
        lipidspace/lipidspace.h \
        lipidspace/AssistanceFunctions.h \
        lipidspace/matplotlibcpp.h \\
        lipidspace/logging.h \
        libraries/cpp-httplib/httplib.h
}