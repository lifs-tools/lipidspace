QT       += core widgets

CONFIG += c++17 debug_and_release
QMAKE_CXXFLAGS += -fopenmp -std=c++17 -march=native -Wno-unknown-pragmas


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

unix {
    LIBS += -fopenmp -Wl,-rpath="'\\\${ORIGIN}'" "-L$$PWD/libraries/cppgoslin/bin/linux64" "-lcppGoslin" "-L$$PWD/libraries/OpenBLAS/bin/linux64" "-lopenblas" "-L$$PWD/libraries/OpenXLSX/bin/linux64" "-lOpenXLSX" "-lssl" "-lcrypto.so"
}

win32 {
    LIBS += -std=c++17 -fopenmp $$PWD\libraries\cppgoslin\bin\win64\libcppGoslin.dll $$PWD\libraries\OpenBLAS\bin\win64\libopenblas.dll $$PWD\libraries\OpenXLSX\bin\win64\libOpenXLSX.dll $$PWD\libraries\openssl\bin\win64\libssl.so $$PWD\libraries\openssl\bin\win64\libcrypto.so
}

macx {
    #LIBS += -fopenmp $$PWD/libraries/cppgoslin/bin/win64/libcppGoslin.dll $$PWD/libraries/OpenBLAS/bin/win64/libopenblas.dll libraries/OpenXLSX/bin/win64/libOpenXLSX.dll
}

INCLUDEPATH += $$PWD/libraries/OpenXLSX/include
DEPENDPATH += $$PWD/libraries/OpenXLSX/include

INCLUDEPATH += $$PWD/libraries/OpenBLAS/include
DEPENDPATH += $$PWD/libraries/OpenBLAS/include

INCLUDEPATH += $$PWD/libraries/cppgoslin
DEPENDPATH += $$PWD/libraries/cppgoslin

INCLUDEPATH += $$PWD/libraries/cpp-httplib
DEPENDPATH += $$PWD/libraries/cpp-httplib

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

release:UI_DIR += objects
release:MOC_DIR += objects
