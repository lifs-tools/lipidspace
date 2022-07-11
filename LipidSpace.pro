!win32:QT       += core gui printsupport widgets charts svg
win32:QT       += core gui printsupport widgets charts svg svgwidgets

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
}

SOURCES += \
    src/about.cpp \
    src/filterdialog.cpp \
    src/importtable.cpp \
    src/selectpc.cpp \
    src/select_worksheet.cpp \
    src/globaldata.cpp \
    src/setalpha.cpp \
    src/CBTableWidget.cpp \
    src/setPCnum.cpp \
    src/progressbar.cpp \
    src/managelipidomes.cpp \
    src/MainWindow.cpp \
    src/lipidspacegui.cpp \
    src/Matrix.cpp \
    src/AssistanceFunctions.cpp \
    src/lipidspace.cpp \
    src/canvas.cpp \
    src/logging.cpp \
    src/statistics.cpp \
    src/Tutorial.cpp \
    src/Chart.cpp \
    src/Boxplot.cpp \
    src/Scatterplot.cpp

HEADERS += \
    lipidspace/about.h \
    lipidspace/filterdialog.h \
    lipidspace/globaldata.h \
    lipidspace/importtable.h \
    lipidspace/lipidspacegui.h \
    lipidspace/CBTableWidget.h \
    lipidspace/canvas.h \
    lipidspace/Matrix.h \
    lipidspace/lipidspace.h \
    lipidspace/AssistanceFunctions.h \
    lipidspace/matplotlibcpp.h \
    lipidspace/managelipidomes.h \
    lipidspace/progressbar.h \
    lipidspace/selectpc.h \
    lipidspace/select_worksheet.h \
    lipidspace/setalpha.h \
    lipidspace/setPCNum.h \
    lipidspace/logging.h \
    lipidspace/statistics.h \
    lipidspace/Tutorial.h \
    lipidspace/Chart.h \
    lipidspace/chartplot.h \
    lipidspace/Boxplot.h \
    lipidspace/Scatterplot.h

FORMS += \
    ui/about.ui \
    ui/filterdialog.ui \
    ui/importtable.ui \
    ui/lipidspacegui.ui \
    ui/managelipidomes.ui \
    ui/progressbar.ui \
    ui/selectpc.ui \
    ui/setalpha.ui \
    ui/setPCnum.ui \
    ui/select_worksheet.ui

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

win32:LIBS += -fopenmp $$PWD/libraries/cppgoslin/bin/win64/libcppGoslin.dll
win32:LIBS += $$PWD/libraries/OpenBLAS/bin/win64/libopenblas.dll
win32:LIBS += -L $$PWD/libraries/OpenXLSX/bin/win64 -lOpenXLSX

win32:INCLUDEPATH += $$PWD/libraries/OpenBLAS/include
win32:DEPENDPATH += $$PWD/libraries/OpenBLAS/include

win32:INCLUDEPATH += $$PWD/libraries/cppgoslin
win32:DEPENDPATH += $$PWD/libraries/cppgoslin

win32:INCLUDEPATH += -fopenmp $$PWD/libraries/OpenXLSX/include
win32:DEPENDPATH += -fopenmp $$PWD/libraries/OpenXLSX/include
