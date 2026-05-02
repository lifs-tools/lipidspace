QT       += core widgets network

CONFIG += c++17 debug_and_release
QMAKE_CXXFLAGS += -std=c++17 -Wno-unknown-pragmas


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

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp -Wl,-rpath="'\\\${ORIGIN}'" "-L$$PWD/libraries/cppgoslin/bin/linux64" "-lcppGoslin" "-L$$PWD/libraries/OpenBLAS/bin/linux64" "-lopenblas" "-L$$PWD/libraries/OpenXLSX/bin/linux64" "-lOpenXLSX" "-lssl" "-lcrypto"

    # Optional CUDA GPU acceleration (Ubuntu 24.04 x86_64, NVIDIA L4 or compatible).
    # Enable at configure time: qmake CONFIG+=cuda_gpu LipidSpaceRest.pro
    # For other GPU generations set CUDA_ARCH accordingly, e.g. sm_86 (A10/A30),
    # sm_80 (A100), sm_90 (H100).
    cuda_gpu {
        DEFINES      += USE_CUDA
        CUDA_DIR      = /usr/local/cuda
        CUDA_ARCH     = sm_89    # NVIDIA L4 (Ada Lovelace)

        INCLUDEPATH  += $$CUDA_DIR/include
        LIBS         += -L$$CUDA_DIR/lib64 -lcudart

        CUDA_SOURCES += src/hausdorff_cuda.cu

        cuda.name             = CUDA compiler
        cuda.input            = CUDA_SOURCES
        cuda.output           = ${QMAKE_FILE_BASE}_cuda.o
        cuda.dependency_type  = TYPE_C
        cuda.variable_out     = OBJECTS
        cuda.commands         = $$CUDA_DIR/bin/nvcc \
            -gencode arch=compute_89,code=[sm_89,compute_89] \
            --compiler-options "-fPIC -O3" \
            -std=c++17 \
            -I"$$PWD" \
            -I"$$CUDA_DIR/include" \
            -DUSE_CUDA \
            -c ${QMAKE_FILE_IN} \
            -o ${QMAKE_FILE_OUT}
        QMAKE_EXTRA_COMPILERS += cuda
    }
}

win32 {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -std=c++17 -fopenmp $$PWD\libraries\cppgoslin\bin\win64\libcppGoslin.dll $$PWD\libraries\OpenBLAS\bin\win64\libopenblas.dll $$PWD\libraries\OpenXLSX\bin\win64\libOpenXLSX.dll $$PWD\libraries\openssl\bin\win64\libssl.so $$PWD\libraries\openssl\bin\win64\libcrypto.so
}

macx {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    INCLUDEPATH += $$system(xcrun --show-sdk-path)/System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers
    INCLUDEPATH += /opt/homebrew/opt/libomp/include
    INCLUDEPATH += /opt/homebrew/opt/openssl@3/include
    LIBS += -L$$PWD/libraries/cppgoslin/bin/macarm64 -lcppGoslin
    LIBS += -L$$PWD/libraries/OpenXLSX/bin/macarm64 -lOpenXLSX
    LIBS += -framework Accelerate
    LIBS += -L/opt/homebrew/opt/libomp/lib -lomp
    LIBS += -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 14.0

    # Bundle dylibs into Contents/Frameworks and patch install names so the
    # app is self-contained and relocatable.
    OPENSSL_LIB = /opt/homebrew/opt/openssl@3/lib
    CRYPTO_INSTALL_NAME = $$system(otool -D $$OPENSSL_LIB/libcrypto.3.dylib | tail -1)
    SSL_INSTALL_NAME    = $$system(otool -D $$OPENSSL_LIB/libssl.3.dylib    | tail -1)
    QMAKE_POST_LINK = \
        mkdir -p $${TARGET}.app/Contents/Frameworks \
        && cp -f $$PWD/libraries/cppgoslin/bin/macarm64/libcppGoslin.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libcppGoslin.dylib $${TARGET}.app/Contents/Frameworks/libcppGoslin.dylib \
        && install_name_tool -change libcppGoslin.dylib @rpath/libcppGoslin.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && cp -f /opt/homebrew/opt/libomp/lib/libomp.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libomp.dylib $${TARGET}.app/Contents/Frameworks/libomp.dylib \
        && install_name_tool -change /opt/homebrew/opt/libomp/lib/libomp.dylib @rpath/libomp.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && cp -fL $${OPENSSL_LIB}/libcrypto.3.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libcrypto.3.dylib $${TARGET}.app/Contents/Frameworks/libcrypto.3.dylib \
        && cp -fL $${OPENSSL_LIB}/libssl.3.dylib $${TARGET}.app/Contents/Frameworks/ \
        && install_name_tool -id @rpath/libssl.3.dylib $${TARGET}.app/Contents/Frameworks/libssl.3.dylib \
        && install_name_tool -change $${CRYPTO_INSTALL_NAME} @rpath/libcrypto.3.dylib $${TARGET}.app/Contents/Frameworks/libssl.3.dylib \
        && install_name_tool -change $${SSL_INSTALL_NAME} @rpath/libssl.3.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && install_name_tool -change $${CRYPTO_INSTALL_NAME} @rpath/libcrypto.3.dylib $${TARGET}.app/Contents/MacOS/$${TARGET} \
        && mkdir -p $${TARGET}.app/Contents/Resources/data \
        && cp $$PWD/data/classes-matrix.csv $${TARGET}.app/Contents/Resources/data/ \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libcppGoslin.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libomp.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libcrypto.3.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/Frameworks/libssl.3.dylib \
        && codesign --force --sign - $${TARGET}.app/Contents/MacOS/$${TARGET}
}

INCLUDEPATH += $$PWD/libraries/OpenXLSX/include
DEPENDPATH += $$PWD/libraries/OpenXLSX/include

# OpenBLAS headers are only needed on non-macOS platforms; macOS uses Accelerate.
!macx {
    INCLUDEPATH += $$PWD/libraries/OpenBLAS/include
    DEPENDPATH += $$PWD/libraries/OpenBLAS/include
}

INCLUDEPATH += $$PWD/libraries/cppgoslin
DEPENDPATH += $$PWD/libraries/cppgoslin

INCLUDEPATH += $$PWD/libraries/nlohmann
DEPENDPATH += $$PWD/libraries/nlohmann

INCLUDEPATH += $$PWD/libraries/cpp-httplib
DEPENDPATH += $$PWD/libraries/cpp-httplib

RESOURCES += lipidspacerest.qrc

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
    lipidspace/matplotlibcpp.h \
    lipidspace/logging.h \
    libraries/cpp-httplib/httplib.h

cuda_gpu {
    HEADERS += lipidspace/hausdorff_cuda.cuh
}

release:UI_DIR += objects
release:MOC_DIR += objects
