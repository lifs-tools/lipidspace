versionAtLeast(QT_VERSION, 6.0.0) {
    QT += core gui printsupport widgets svg svgwidgets network
}
!versionAtLeast(QT_VERSION, 6.0.0) {
    QT += core gui printsupport widgets svg network
}


CONFIG += c++17 debug_and_release
!win32-msvc*: QMAKE_CXXFLAGS += -std=c++17 -Wno-unknown-pragmas

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

!win32-msvc*: QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_EXTRA_TARGETS += build buildclean

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp -Wl,-rpath="'\\\${ORIGIN}'" "-L$$PWD/libraries/cppgoslin/bin/linux64" "-lcppGoslin" "-L$$PWD/libraries/OpenBLAS/bin/linux64" "-lopenblas" "-L$$PWD/libraries/OpenXLSX/bin/linux64" "-lOpenXLSX"
}

win32-msvc* {
    QMAKE_CXXFLAGS += /openmp
    LIBS += $$PWD\libraries\cppgoslin\bin\win64\libcppGoslin.dll \
            $$PWD\libraries\OpenBLAS\bin\win64\libopenblas.dll \
            $$PWD\libraries\OpenXLSX\bin\win64\libOpenXLSX.dll
}

win32-g++ {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp \
            $$PWD\libraries\cppgoslin\bin\win64\libcppGoslin.dll \
            $$PWD\libraries\OpenBLAS\bin\win64\libopenblas.dll \
            $$PWD\libraries\OpenXLSX\bin\win64\libOpenXLSX.dll
}

macx {
    ICON = LipidSpace.icns
    QMAKE_TARGET_BUNDLE_PREFIX = org.lifs-tools
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    INCLUDEPATH += $$system(xcrun --show-sdk-path)/System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers
    INCLUDEPATH += /opt/homebrew/opt/libomp/include
    LIBS += -L$$PWD/libraries/cppgoslin/bin/macarm64 -lcppGoslin
    LIBS += -L$$PWD/libraries/OpenXLSX/bin/macarm64 -lOpenXLSX
    LIBS += -framework Accelerate
    LIBS += -L/opt/homebrew/opt/libomp/lib -lomp
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 14.0

    # Bundle dylibs into Contents/Frameworks and patch install names so the
    # app is self-contained and relocatable.
    QMAKE_POST_LINK = \
        mkdir -p $${TARGET}.app/Contents/Frameworks \
        && cp -f $$PWD/libraries/cppgoslin/bin/macarm64/libcppGoslin.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libcppGoslin.dylib $${TARGET}.app/Contents/Frameworks/libcppGoslin.dylib \
        && install_name_tool -change libcppGoslin.dylib @rpath/libcppGoslin.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && cp -f /opt/homebrew/opt/libomp/lib/libomp.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libomp.dylib $${TARGET}.app/Contents/Frameworks/libomp.dylib \
        && install_name_tool -change /opt/homebrew/opt/libomp/lib/libomp.dylib @rpath/libomp.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && mkdir -p $${TARGET}.app/Contents/Resources/data \
        && cp $$PWD/data/classes-matrix.csv $${TARGET}.app/Contents/Resources/data/ \
        && cp -r $$PWD/data/images $${TARGET}.app/Contents/Resources/data/ \
        && cp -r $$PWD/examples $${TARGET}.app/Contents/Resources/ \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libcppGoslin.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libomp.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/MacOS/$${TARGET}
}

INCLUDEPATH += $$PWD/libraries/OpenXLSX/include
DEPENDPATH += $$PWD/libraries/OpenXLSX/include

INCLUDEPATH += $$PWD/libraries/OpenBLAS/include
DEPENDPATH += $$PWD/libraries/OpenBLAS/include

INCLUDEPATH += $$PWD/libraries/cppgoslin
DEPENDPATH += $$PWD/libraries/cppgoslin

INCLUDEPATH += $$PWD/libraries/nlohmann
DEPENDPATH += $$PWD/libraries/nlohmann

#INCLUDEPATH += $$PWD/libraries/cpp-httplib
#DEPENDPATH += $$PWD/libraries/cpp-httplib

#INCLUDEPATH += $$PWD/libraries/openssl
#DEPENDPATH += $$PWD/libraries/openssl

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
    src/studyvariablemapping.cpp \
    src/changecolordialog.cpp

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
    lipidspace/studyvariablemapping.h \
    lipidspace/changecolordialog.h

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
    ui/studyvariablemapping.ui \
    ui/changecolordialog.ui

RESOURCES += lipidspace.qrc

release:UI_DIR += objects
release:MOC_DIR += objects

unix:!macx {
    build.depends = release
    build.commands += mkdir -p $$PWD/Build
    build.commands += && mkdir -p $$PWD/Build/LipidSpace
    build.commands += && mkdir -p $$PWD/Build/LipidSpace/data
    build.commands += && mkdir -p $$PWD/Build/LipidSpace/examples
    build.commands += && cp $$PWD/LipidSpace $$PWD/Build/LipidSpace
    build.commands += && cp $$PWD/libraries/cppgoslin/bin/linux64/* $$PWD/Build/LipidSpace
    build.commands += && cp $$PWD/libraries/OpenBLAS/bin/linux64/* $$PWD/Build/LipidSpace
    build.commands += && cp $$PWD/libraries/OpenXLSX/bin/linux64/* $$PWD/Build/LipidSpace
    build.commands += && cp $$PWD/libraries/Qt/bin/linux64/* $$PWD/Build/LipidSpace
    build.commands += && cp $$PWD/data/classes-matrix.csv $$PWD/Build/LipidSpace/data
    build.commands += && cp -r $$PWD/data/images $$PWD/Build/LipidSpace/data
    build.commands += && cp $$PWD/examples/Example-Dataset.xlsx $$PWD/Build/LipidSpace/examples
    build.commands += && cp $$PWD/examples/ThreeStudies.xlsx $$PWD/Build/LipidSpace/examples
    build.commands += && cp $$PWD/LICENSE* $$PWD/Build/LipidSpace
    build.commands += && cd $$PWD/Build && zip -9 -r LipidSpace.zip LipidSpace

    buildclean.depends = clean
    buildclean.commands += rm -rf $$PWD/Build
}

win32 {
	#build.depends = release
	build.commands += .\win-build.bat

    #buildclean.depends = clean
    buildclean.commands += del /f \"$$PWD/Build\"
}
