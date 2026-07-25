QT += core testlib
QT -= gui

CONFIG += c++17 console testcase
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++17 -Wno-unknown-pragmas

TEMPLATE = app
TARGET = tst_matrix_blas

# Repo root (this .pro lives in tests/tst_matrix_blas/).
ROOT = $$PWD/../..

# Headers needed to compile Matrix.cpp (cppgoslin/nlohmann are header-only here;
# Matrix.cpp references no symbols from them, so nothing extra to link).
INCLUDEPATH += $$ROOT
INCLUDEPATH += $$ROOT/libraries/cppgoslin
INCLUDEPATH += $$ROOT/libraries/nlohmann

macx {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    INCLUDEPATH += /opt/homebrew/opt/libomp/include
    INCLUDEPATH += $$system(xcrun --show-sdk-path)/System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers
    LIBS += -framework Accelerate
    LIBS += -L/opt/homebrew/opt/libomp/lib -lomp
}
unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    INCLUDEPATH += $$ROOT/libraries/OpenBLAS/include
    # Embed the vendored OpenBLAS dir as an rpath so the test binary runs in CI
    # without LD_LIBRARY_PATH (OpenBLAS also supplies the LAPACK dgetrf_/dgetri_).
    LIBS += -fopenmp -Wl,-rpath,$$ROOT/libraries/OpenBLAS/bin/linux64 -L$$ROOT/libraries/OpenBLAS/bin/linux64 -lopenblas
}

HEADERS += $$ROOT/lipidspace/Matrix.h
SOURCES += \
    $$ROOT/src/Matrix.cpp \
    tst_matrix_blas.cpp
