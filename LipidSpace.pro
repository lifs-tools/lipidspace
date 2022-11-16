versionAtLeast(QT_VERSION, 6.0.0) {
    QT += core gui printsupport widgets svg svgwidgets
}
!versionAtLeast(QT_VERSION, 6.0.0) {
    QT += core gui printsupport widgets svg
}

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
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS_RELEASE += -O3

unix {
    LIBS += -fopenmp -Wl,-rpath="'\\\${ORIGIN}'" "-L$$PWD/libraries/cppgoslin/bin/linux64" "-lcppGoslin" "-L$$PWD/libraries/OpenBLAS/bin/linux64" "-lopenblas" "-L$$PWD/libraries/OpenXLSX/bin/linux64" "-lOpenXLSX"
}

win32 {
    LIBS += -fopenmp $$PWD\libraries\cppgoslin\bin\win64\libcppGoslin.dll $$PWD\libraries\OpenBLAS\bin\win64\libopenblas.dll $$PWD\libraries\OpenXLSX\bin\win64\libOpenXLSX.dll
}

macx {
    #LIBS += -fopenmp $$PWD/libraries/cppgoslin/bin/win64/libcppGoslin.dll $$PWD/libraries/OpenBLAS/bin/win64/libopenblas.dll libraries/OpenXLSX/bin/win64/libOpenXLSX.dll
}

INCLUDEPATH += -fopenmp $$PWD/libraries/OpenXLSX/include
DEPENDPATH += -fopenmp $$PWD/libraries/OpenXLSX/include

INCLUDEPATH += $$PWD/libraries/OpenBLAS/include
DEPENDPATH += $$PWD/libraries/OpenBLAS/include

INCLUDEPATH += $$PWD/libraries/cppgoslin
DEPENDPATH += $$PWD/libraries/cppgoslin


SOURCES += \
    src/about.cpp \
    src/filterdialog.cpp \
    src/importtable.cpp \
    src/selectlipidomes.cpp \
    src/selectpc.cpp \
    src/select_worksheet.cpp \
    src/globaldata.cpp \
    src/setalpha.cpp \
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
    src/chart.cpp \
    src/chartplot.cpp \
    src/plots.cpp \
    src/studyvariablemapping.cpp

HEADERS += \
    lipidspace/about.h \
    lipidspace/filterdialog.h \
    lipidspace/globaldata.h \
    lipidspace/importtable.h \
    lipidspace/lipidspacegui.h \
    lipidspace/canvas.h \
    lipidspace/Matrix.h \
    lipidspace/lipidspace.h \
    lipidspace/AssistanceFunctions.h \
    lipidspace/matplotlibcpp.h \
    lipidspace/managelipidomes.h \
    lipidspace/progressbar.h \
    lipidspace/selectlipidomes.h \
    lipidspace/selectpc.h \
    lipidspace/select_worksheet.h \
    lipidspace/setalpha.h \
    lipidspace/setPCNum.h \
    lipidspace/logging.h \
    lipidspace/statistics.h \
    lipidspace/Tutorial.h \
    lipidspace/chart.h \
    lipidspace/chartplot.h \
    lipidspace/plots.h \
    lipidspace/studyvariablemapping.h

FORMS += \
    ui/about.ui \
    ui/filterdialog.ui \
    ui/importtable.ui \
    ui/lipidspacegui.ui \
    ui/managelipidomes.ui \
    ui/progressbar.ui \
    ui/selectlipidomes.ui \
    ui/selectpc.ui \
    ui/setalpha.ui \
    ui/setPCnum.ui \
    ui/select_worksheet.ui \
    ui/studyvariablemapping.ui

release:UI_DIR += objects
release:MOC_DIR += objects

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


