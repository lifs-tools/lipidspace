# OpenXLSX — vendored source revision

Everything under this directory (`include/` **and** every `bin/<platform>/` archive)
must come from **one and the same** upstream revision.

    Upstream:  https://codeberg.org/troldal/OpenXLSX
               (was https://github.com/troldal/OpenXLSX until v0.5.1, June 2026)
    Revision:  9d673a34e59e156cc3477a8fdb70d55a91bb8646   (2025-04-20)

## Why this file exists

`IZipArchive` is a header-only, type-erased interface: the vtable layout lives in
`include/headers/IZipArchive.hpp`, but the calls through it are compiled into the
library. If the headers and the library come from different revisions, the build
still links cleanly and then jumps through a wrong vtable slot at runtime — the
first `XLDocument` open segfaults with no error message.

That is exactly what happened before: the Windows binaries were rebuilt from a
newer upstream revision than the vendored headers. Upstream had inserted
`addEntryAndCommit` into `IZipArchive::Concept` between `addEntry` and
`deleteEntry`, shifting `getEntry`/`hasEntry` down one slot each. Every XLSX
import on Windows crashed silently.

## Link statically

Link OpenXLSX **statically on every platform**, as `LipidSpace.pro` now does.
A static archive cannot drift away from the headers it was built with, and it
keeps allocation and exception handling inside a single module. Upstream gives
the same advice: *"On Windows, it is much easier to just include the OpenXLSX
source folder as a subdirectory to your CMake project; it will save you a lot of
headaches."*

Do not reintroduce `BUILD_SHARED_LIBS=ON` here.

## Rebuilding

Build all platforms from the revision above, and update the revision here and
the headers in `include/` in the *same* commit as the archives in `bin/`.

    git clone https://codeberg.org/troldal/OpenXLSX.git
    cd OpenXLSX
    git checkout 9d673a34e59e156cc3477a8fdb70d55a91bb8646

    cmake -S . -B build-static -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DOPENXLSX_CREATE_DOCS=OFF \
        -DOPENXLSX_BUILD_SAMPLES=OFF \
        -DOPENXLSX_BUILD_TESTS=OFF \
        -DOPENXLSX_BUILD_BENCHMARKS=OFF \
        -DCMAKE_INSTALL_PREFIX=/tmp/openxlsx-install
    cmake --build build-static -j
    cmake --install build-static

Then copy:

  * `/tmp/openxlsx-install/lib/libOpenXLSX.a` → `bin/<platform>/`
  * `/tmp/openxlsx-install/include/OpenXLSX/` → `include/`

On Windows use the MinGW toolchain that ships with Qt
(`C:\Qt\Tools\mingw1310_64\bin`), so the archive matches the compiler used for
LipidSpace itself.

At this revision miniz, nowide and pugixml are compiled into `libOpenXLSX.a`;
there are no separate archives to copy. Later upstream revisions split them out
into `lib/OpenXLSX/lib{miniz,nowide,pugixml}.a` — if you move the pin forward,
copy those too and add them to `LipidSpace.pro` *after* `libOpenXLSX.a`.

## Verifying a rebuild

Before committing, check that the headers and the archive still agree:

    g++ -std=c++17 -I include check.cpp bin/<platform>/libOpenXLSX.a -o check

where `check.cpp` opens `examples/Example-Dataset.xlsx` and iterates the "Data"
worksheet. It must read 33 rows of 369 columns. A segfault inside
`XLDocument::extractXmlFromArchive` means the headers and the archive are out of
sync again.
