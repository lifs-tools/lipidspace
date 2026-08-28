QT       += core widgets network

CONFIG += c++17 debug_and_release
QMAKE_CXXFLAGS += -std=c++17 -Wno-unknown-pragmas


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Enable cpp-httplib transparent gzip (Content-Encoding / Accept-Encoding). Lets clients
# store and exchange the Atlas artifact as gzipped JSON and shrinks all REST payloads.
# Backward-compatible: httplib only (de)compresses when the peer opts in via the headers.
DEFINES += CPPHTTPLIB_ZLIB_SUPPORT

RC_ICONS = LipidSpace.ico

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp -Wl,-rpath="'\\\${ORIGIN}'" "-L$$PWD/libraries/cppgoslin/bin/linux64" "-lcppGoslin" "-L$$PWD/libraries/OpenBLAS/bin/linux64" "-lopenblas" "-L$$PWD/libraries/OpenXLSX/bin/linux64" "-lOpenXLSX" "-lssl" "-lcrypto" "-lz"

    # Optional CUDA GPU acceleration (Linux x86_64). Enable at configure time:
    #     qmake CONFIG+=cuda_gpu LipidSpaceRest.pro
    #
    # Fat binary: native SASS for our data-centre fleet + one PTX image for forward
    # compatibility (the driver JIT-compiles it on any GPU newer than the PTX arch).
    #   T4 = sm_75 · A100 = sm_80 · A40 = sm_86 · L4 = sm_89 · H100/H200 = sm_90
    #   B200 / GB200 / B300 (data-centre Blackwell) = sm_100  — requires CUDA Toolkit >= 12.8
    # Build with the newest toolkit that covers all target archs (>= 12.8 for Blackwell);
    # if building with CUDA < 12.8, delete the two sm_100 / compute_100 lines below
    # (Blackwell then still runs, JIT-compiled from the compute_90 PTX, just not native SASS).
    #
    # Static CUDA runtime (--cudart=static + libcudart_static): the deployed binary needs
    # only the NVIDIA driver (libcuda.so.1) on the target host — NOT the CUDA toolkit.
    cuda_gpu {
        DEFINES      += USE_CUDA
        CUDA_DIR      = /usr/local/cuda

        INCLUDEPATH  += $$CUDA_DIR/include
        # Static cudart + its support libs; libcuda.so.1 (the driver) is resolved at runtime.
        LIBS         += -L$$CUDA_DIR/lib64 -lcudart_static -lculibos -lrt -lpthread -ldl

        CUDA_SOURCES += src/hausdorff_cuda.cu

        cuda.name             = CUDA compiler
        cuda.input            = CUDA_SOURCES
        cuda.output           = ${QMAKE_FILE_BASE}_cuda.o
        cuda.dependency_type  = TYPE_C
        cuda.variable_out     = OBJECTS
        cuda.commands         = $$CUDA_DIR/bin/nvcc \
            -gencode arch=compute_75,code=sm_75 \
            -gencode arch=compute_80,code=sm_80 \
            -gencode arch=compute_86,code=sm_86 \
            -gencode arch=compute_89,code=sm_89 \
            -gencode arch=compute_90,code=sm_90 \
            -gencode arch=compute_100,code=sm_100 \
            -gencode arch=compute_100,code=compute_100 \
            --cudart=static \
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
    LIBS += -lz
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
    src/atlas.cpp \
    src/logging.cpp \
    src/lipidspacerest.cpp
HEADERS += \
    lipidspace/globaldata.h \
    lipidspace/Matrix.h \
    lipidspace/lipidspace.h \
    lipidspace/AssistanceFunctions.h \
    lipidspace/atlas.h \
    lipidspace/matplotlibcpp.h \
    lipidspace/logging.h \
    libraries/cpp-httplib/httplib.h

cuda_gpu {
    HEADERS += lipidspace/hausdorff_cuda.cuh
}

release:UI_DIR += objects
release:MOC_DIR += objects
