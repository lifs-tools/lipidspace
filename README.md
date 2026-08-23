# LipidSpace

[![CI Status](https://github.com/lifs-tools/lipidspace/actions/workflows/build.yml/badge.svg)](https://github.com/lifs-tools/lipidspace/actions/workflows/build.yml)

LipidSpace is a stand-alone tool to analyze and compare lipidomes by assessing their structural differences. A graph-based comparison of lipid structures allows to calculate distances between lipids and to determine similarities across lipidomes. It allows for a rapid (re)analysis of experiments, identifies lipids responsible for shaping the respective lipidome, and provides methods for quality
control.

LipidSpace is built and tested on Windows 10/11, Ubuntu 22.04 and 24.04, and macOS 12+ (ARM64 / Apple Silicon). It comes with four built-in tutorials to get you started.

Please check the `LICENSE*` files for more information about individual license terms of embedded libraries.

## Installing a release

Most users do not need to build anything. Go to the [download page](https://lifs-tools.org/lipidspace.html), pick the archive for your operating system and unpack it anywhere — there is no installer.

| Platform | Start with |
|---|---|
| Windows | `LipidSpace.exe` |
| Linux | `LipidSpace.sh` |
| macOS | `LipidSpace.app` (see [macOS Gatekeeper](#macos-gatekeeper-and-code-signing)) |

The archive is self-contained: Qt, the analysis libraries, the example datasets and the tutorial data all travel with it.

## Frequently asked questions

<details><summary><b>How can I install LipidSpace?</b></summary>
<p>
Please go on the <a href="https://lifs-tools.org/lipidspace.html" target="_blank">download page</a> and download the right version for your operation system. The download comes as a zip folder. Just unzip the folder on your computer and start LipidSpace.exe or LipidSpace.sh. No installation is necessary.
</p>
</details>

<details><summary><b>What data formats can be imported?</b></summary>
<p>
LipidSpace supports both csv and xlsx files for import. The tabels can be either pivot tables (that is lipid species in columns, samples in rows or lipid species in rows and samples in columns) or flat tables. Further, lipidomics data files in <a href=https://pubs.acs.org/doi/10.1021/acs.analchem.8b04310>mzTab-M</a> format can be imported.
</p>
</details>

<details><summary><b>How can I play around with LipidSpace?</b></summary>
<p>
You can open our provided example dataset in the menu → File → Import example dataset.
</p>
</details>

<details><summary><b>How can I quickly select a subset of lipidomes for a follow-up analysis?</b></summary>
<p>
Go to the dendrogram view and right-click on any branch which you are interested. In the context menu choose 'Select these lipidomes in sample selection' and restart the analysis.
</p>
</details>

<details><summary><b>How can I remove individual lipidomes from the analysis?</b></summary>
<p>
If you want to remove lipidomes only temporaly, simply deselect them in the sample tab on the left-hand side. If you want to remove lipidomes permantently, go in the menu → Analysis → Manage lipidomes.
</p>
</details>

<details><summary><b>What is a 'Complete feature analysis'?</b></summary>
<p>
When your imported data contains at least two study variables, LipidSpace creates for each study variable a regression/classification model and applies this model on all remaining study variables to compute the accuracy. This is a meaure for the relation/dependency between any two study variables. You can run and store such an analysis in the menu → Analysis → Complete feature analysis.
</p>
</details>

<details><summary><b>Can I save my current analysis?</b></summary>
<p>
At the moment it is not possible to store or load an analysis. But please drop us a line if you think that this feature is necessary.
</p>
</details>

<details><summary><b>Which fatty acyl chain is compared to which one?</b></summary>
<p>
In default mode, LipidSpace is comparing the first fatty acyl chain (FA) of the first lipid with the first FA of the second lipid, the second FA of the first lipid with the second FA of the second lipid, etc. However, when the sn-position is not specified as for instance in PC 18:0_16:1, a mode can be activated to compare all combinations of FA comparisons for both lipids and picking the lowest distance. The results are more accurate, but the performance is decreased. You can activate this mode in the menu → Analysis → Ignore lipid sn-positions.
</p>
</details>

<details><summary><b>What is an unbound lipid distance metric?</b></summary>
<p>
As default, LipidSpace is using a bound distance metric to compare the structure of any two lipids. That means that the distance is a value that ranges between 0 (both lipids are identical) and 1. However, other distance measures suggest an unbound distance ranging from 0 to infinity. This mode provides more accurate results but reduces the visibility of the lipid spaces since the distances may become very big. You can activate this mode in the Menu → Analysis → Unbound distance metric.
</p>
</details>

<details><summary><b>Which linkage types are supported for creating the lipidome dendrogram?</b></summary>
<p>
The user can switch between single linkage, unweighted average, and complete linkage clustering in the menu → Analysis → Clustering strategy.
</p>
</details>

<details><summary><b>Where can I find a tutorial for LipidSpace?</b></summary>
<p>
Directly when opening LipidSpace, you can start four interactive tutorials within the tool that guide you through the actual user interface. The tutorials are give an introduction to i) data import, ii) handling of the UI for result interpretation, iii) feature analysis, and iv) quality control methods.
</p>
</details>

<details><summary><b>Are there easter eggs hidden in LipidSpace?</b></summary>
<p>
Of course, feel free to spot them ;-)
</p>
</details>

<details><summary><b>Some feature in your tool is missing for my analyses? Can I ask you for a feature request?</b></summary>
<p>
We're delighted when someone gives us feedback on LipidSpace, even when it is a feature request. Therefore, yes please :-)
</p>
</details>

# Building from source

## Getting the source

```bash
git clone https://github.com/lifs-tools/lipidspace.git
cd lipidspace
```

Clone with full history rather than `--depth 1`: the build reads the most recent tag to stamp the version into the binary (see [Version stamping](#version-stamping)).

## Bundled dependencies

Everything except Qt ships with the repository under `libraries/`, so there is nothing else to install or configure:

| Library | Windows | Linux | macOS ARM64 |
|---|---|---|---|
| [cppgoslin](https://github.com/lifs-tools/cppgoslin) — lipid name parsing | `libcppGoslin.dll` | `libcppGoslin.so` | `libcppGoslin.dylib` |
| OpenBLAS — linear algebra | `libopenblas.dll` | `libopenblas.so` | Apple Accelerate framework |
| [OpenXLSX](https://codeberg.org/troldal/OpenXLSX) — `.xlsx` import/export | `libOpenXLSX.a` | `libOpenXLSX.a` | `libOpenXLSX.a` |
| nlohmann/json | header-only | header-only | header-only |

OpenXLSX is linked **statically on every platform**. Do not switch it to a shared library — its `IZipArchive` interface puts the vtable layout in the headers, so a header/binary mismatch links cleanly and then segfaults on the first spreadsheet you open. See [`libraries/OpenXLSX/PROVENANCE.md`](libraries/OpenXLSX/PROVENANCE.md).

Qt is the only prerequisite you install yourself. LipidSpace uses [Qt 6](https://www.qt.io/product/qt6) (`core gui widgets printsupport svg svgwidgets network`).

## Windows

LipidSpace is built with **Qt 6 and the MinGW toolchain**. MSVC is not supported: the bundled cppgoslin and OpenBLAS binaries are MinGW builds, and the project relies on GCC's OpenMP.

### Setting up the toolchain

Install Qt with the [Qt Online Installer](https://www.qt.io/download-qt-installer) and select, under the Qt 6 version you want:

* **MinGW 64-bit** (the Qt build itself), and
* under *Developer and Designer Tools* → **MinGW 13.1.0 64-bit** (the compiler).

Then put both on your `PATH`, adjusting the Qt version to what you installed:

```bat
set PATH=C:\Qt\6.11.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%
```

Check that the toolchain resolves to the Qt you just installed — mixing a second MinGW or Qt from elsewhere on `PATH` is the most common cause of confusing Windows build failures:

```bat
where qmake6
where g++
where mingw32-make
```

### Building

```bat
qmake6 LipidSpace.pro
mingw32-make release
```

The executable lands in `release\LipidSpace.exe`.

Qt Creator works too — open `LipidSpace.pro` directly. It builds into a shadow directory such as `build\Desktop_Qt_6_11_0_MinGW_64_bit-Release\release\`, which the packaging script finds on its own.

### Packaging

```bat
.\win-build.bat
```

This assembles `Build\LipidSpace\` and zips it to `Build\LipidSpace.zip`. It locates the freshly built executable (in-tree or in a Qt Creator shadow build), copies the bundled DLLs, runs `windeployqt` to pull in Qt and its plugins, and adds the data files, examples and licences. Pass an explicit path if you want to package a specific binary:

```bat
.\win-build.bat path\to\LipidSpace.exe
```

`windeployqt` is deliberately taken from the directory holding `qmake`, not from `PATH`, so the deployed Qt plugins always match the Qt the executable was built against.

To clean up all build output:

```bat
.\win-clean.bat
```

## Linux

Tested on Ubuntu 22.04 and 24.04.

```bash
sudo apt install \
  build-essential \
  libfontconfig1 \
  qt6-base-dev \
  qt6-base-dev-tools \
  libqt6svg6-dev \
  libqt6svgwidgets6 \
  libopenblas-dev \
  libomp-dev \
  mesa-common-dev \
  libglu1-mesa-dev \
  libc6 \
  libstdc++6
```

Add `libssl-dev` as well if you intend to build LipidSpace REST.

### Building

```bash
qmake6 LipidSpace.pro
make -j"$(nproc)"
```

The binary is linked with an `$ORIGIN` rpath, so it looks for `libcppGoslin.so` and `libopenblas.so` next to itself. The simplest way to run it is to build the distribution folder, which puts everything in one place, and start it through the launcher script:

```bash
make build
cd Build/LipidSpace
./LipidSpace.sh
```

`LipidSpace.sh` adds the folder to `LD_LIBRARY_PATH` and sets `XDG_SESSION_TYPE` before launching the binary — start it this way rather than running `./LipidSpace` directly.

To run the binary straight from the source tree instead, point the loader at the bundled libraries:

```bash
LD_LIBRARY_PATH=libraries/cppgoslin/bin/linux64:libraries/OpenBLAS/bin/linux64 ./LipidSpace
```

## macOS (ARM64 / Apple Silicon)

Install the Xcode command line tools and the Homebrew dependencies:

```bash
xcode-select --install
brew install qt libomp openssl@3 cmake git
```

Put Qt 6 on your `PATH` so `qmake6` is found — add this to `~/.zshrc` or run it in the shell you build from:

```bash
export PATH="/opt/homebrew/opt/qt/bin:$PATH"
```

### Building the OpenXLSX dependency

The macOS archive is not committed to the repository and has to be built once:

```bash
chmod +x build-openxlsx-macos-arm64.sh
./build-openxlsx-macos-arm64.sh
```

The script checks out the exact OpenXLSX revision the vendored headers came from, builds it as a static `arm64` library, writes it to `libraries/OpenXLSX/bin/macarm64/libOpenXLSX.a`, and then verifies that the archive and the headers agree by reading the example dataset. Do not point it at a different upstream revision without also replacing `libraries/OpenXLSX/include/` — see [`libraries/OpenXLSX/PROVENANCE.md`](libraries/OpenXLSX/PROVENANCE.md).

### Building

```bash
qmake6 LipidSpace.pro
make release
```

This produces `LipidSpace.app`. The build copies `libcppGoslin.dylib` and `libomp.dylib` into `Contents/Frameworks`, rewrites their install names, and copies the data files and examples into `Contents/Resources`, so the bundle is self-contained and relocatable. OpenBLAS is not needed — macOS uses Apple's Accelerate framework instead.

### macOS Gatekeeper and code signing

**A macOS build has to be signed, or macOS refuses to launch it.**

#### "Apple could not verify LipidSpace is free of malware"

This is what you get from any artifact downloaded from a browser — including the archives produced by the release workflow. Those are **not notarized**, and macOS blocks unnotarized downloads regardless of how the bundle is signed. It is not a sign that the download is broken.

Unblock it by removing the quarantine attribute that the browser attached:

```bash
xattr -dr com.apple.quarantine /Applications/LipidSpace.app
```

For a `.dmg`, clear it on the mounted app after copying it out, or on the `.dmg` before opening it. Right-clicking the app and choosing *Open* also works on some macOS versions, and *System Settings → Privacy & Security → Open Anyway* offers the same override after a blocked launch.

#### Signing your own build

`make release` ad-hoc signs the two bundled dylibs and the main binary, but not the bundle as a whole. If you modify the bundle afterwards — or assemble a `.dmg` from it — re-sign it:

```bash
codesign --force --deep --sign - LipidSpace.app
codesign --verify --deep --strict --verbose=2 LipidSpace.app
```

An ad-hoc signature (`--sign -`) is enough for a bundle you built yourself and never sent through a browser.

#### Distributing to other people

To hand someone a `.dmg` that opens without any of the steps above, it must be signed with a Developer ID Application certificate **and** notarized by Apple. Both are required; signing alone is not enough.

```bash
codesign --force --deep --options runtime --timestamp \
    --sign "Developer ID Application: YOUR NAME (TEAMID)" LipidSpace.app
# ...build the .dmg from the signed bundle, then:
xcrun notarytool submit LipidSpace.dmg --keychain-profile "AC_PASSWORD" --wait
xcrun stapler staple LipidSpace.dmg
```

Stapling matters: it embeds the notarization ticket so the `.dmg` opens on machines that are offline or behind a proxy. Check how Gatekeeper actually judges the result before shipping:

```bash
spctl -a -t exec -vv LipidSpace.app
```

This needs an Apple Developer Program membership. Until the release workflow is given a signing certificate and an App Store Connect key, macOS users have to clear the quarantine attribute themselves — worth stating on the download page.

## Version stamping

The version is derived once, in [`version.pri`](version.pri), and compiled into both LipidSpace and LipidSpace REST as `LIPIDSPACE_VERSION`. It is what the About box, the window title and the REST service report. Precedence:

1. `$RELEASE_VERSION` — set by the release workflow from the git tag, and by the Docker build argument
2. `git describe --tags --always --dirty` — e.g. `v1.2.1-6-gbcbb2b77a` for local builds
3. `LIPIDSPACE_FALLBACK_VERSION` from `version.pri` — for builds from a source tarball with no git available

`qmake` prints the version it resolved, so you can confirm it before building:

```
Project MESSAGE: LipidSpace version: v1.2.1-6-gbcbb2b77a
```

To build as a specific version without tagging:

```bash
RELEASE_VERSION=v1.3.0 qmake6 LipidSpace.pro
```

Do not hard-code the version anywhere else.

## Packaging a distribution

| Platform | Commands | Result |
|---|---|---|
| Linux | `qmake6 LipidSpace.pro && make build` | `Build/LipidSpace.zip` |
| Windows | `qmake6 LipidSpace.pro && mingw32-make release` then `.\win-build.bat` | `Build\LipidSpace.zip` |
| macOS | `qmake6 LipidSpace.pro && make release` then `ditto -c -k --sequesterRsrc --keepParent LipidSpace.app LipidSpace.zip` | `LipidSpace.zip` |

A source archive can be produced on any platform with:

```bash
qmake6 LipidSpace.pro
make dist
```

## Rebuilding the vendored OpenXLSX

Read [`libraries/OpenXLSX/PROVENANCE.md`](libraries/OpenXLSX/PROVENANCE.md) first. The headers in `libraries/OpenXLSX/include/` and every `libraries/OpenXLSX/bin/*/libOpenXLSX.a` must come from one and the same upstream revision, and must be updated in the same commit. A mismatch is invisible to the compiler and the linker, and shows up as a silent crash the first time a spreadsheet is opened.

`tests/xlsx_smoke.cpp` guards this. It runs in CI on all three platforms and can be run by hand:

```bash
g++ -std=c++17 -I libraries/OpenXLSX/include tests/xlsx_smoke.cpp \
    libraries/OpenXLSX/bin/linux64/libOpenXLSX.a -o xlsx_smoke
./xlsx_smoke examples/Example-Dataset.xlsx     # must print 33 rows x 369 columns
```

## Troubleshooting

<details><summary><b>Build fails with a missing <code>lipidspace/CBTableWidget.h</code> or a cppgoslin signature error</b></summary>
<p>

```
./ui_lipidspacegui.h:33:10: fatal error: lipidspace/CBTableWidget.h: No such file or directory
```

or

```
src/lipidspace.cpp:308:90: error: no matching function for call to ‘LipidAdduct::LipidAdduct(LipidAdduct*&)’
```

Make sure you have the latest version of [cppgoslin](https://github.com/lifs-tools/cppgoslin) installed on your computer (`make && sudo make install`).
</p>
</details>

<details><summary><b>Windows: <code>windeployqt</code> reports "Unable to find the platform plugin"</b></summary>
<p>
Usually this means a second Qt or MinGW installation is ahead of the intended one on <code>PATH</code>. Check <code>where qmake6</code>, <code>where g++</code> and <code>where windeployqt6</code>, and note that <code>win-build.bat</code> prints the exact <code>windeployqt</code> and <code>qmake</code> it selected. Do not add <code>--release</code> to the <code>windeployqt</code> call — with MinGW builds it causes exactly this error by discarding every plugin.
</p>
</details>

<details><summary><b>Any platform: LipidSpace starts but crashes when opening an <code>.xlsx</code> file</b></summary>
<p>
The vendored OpenXLSX headers and static library are out of sync. Run the smoke test above; if it segfaults, rebuild the archive from the revision recorded in <code>libraries/OpenXLSX/PROVENANCE.md</code>.
</p>
</details>

# LipidSpace REST

`LipidSpaceRest` exposes the analysis over HTTP, and is what the Docker image runs.

## Building

```bash
qmake6 LipidSpaceRest.pro
make -j"$(nproc)"
```

On Linux this additionally needs `libssl-dev`. On macOS, OpenSSL comes from Homebrew's `openssl@3` and is picked up automatically; copy the bundled dylib next to the executable before running it:

```bash
cp libraries/cppgoslin/bin/macarm64/libcppGoslin.dylib .
./LipidSpaceRest
```

### Optional CUDA acceleration

The Hausdorff distance calculation can run on an NVIDIA GPU:

```bash
qmake6 CONFIG+=cuda_gpu LipidSpaceRest.pro
```

This requires an NVIDIA L4 family GPU and the CUDA libraries. For a different GPU, adjust the architecture in `LipidSpaceRest.pro`:

```
CUDA_ARCH     = sm_89    # NVIDIA L4 (Ada Lovelace)
```

## Running

```bash
./LipidSpaceRest
```

| Option | Default | Meaning |
|---|---|---|
| `-b`, `--bind <addr>` | `0.0.0.0` | Address to bind to |
| `-p`, `--port <port>` | `8888` | Port to listen on |
| `-t`, `--tmp_folder <path>` | `.` | Working directory for per-request temporary data |
| `-d`, `--debug` | off | Save incoming and outgoing JSON requests |
| `-k`, `--disk-threshold-mb <mb>` | `10` | Free space below which `/actuator/health` reports `DOWN` |
| `-i`, `--vacuum-interval-secs <s>` | `60` | How often the temp folder is swept |
| `-a`, `--vacuum-max-age-secs <s>` | `3600` | Age at which a temp directory is removed unconditionally |
| `-m`, `--vacuum-min-active-secs <s>` | `300` | Minimum age before a temp directory may be removed under disk pressure |
| `--version` | | Print the version and exit |
| `--help` | | Print all options and exit |

### Endpoints

| Method | Path | Purpose |
|---|---|---|
| `POST` | `/lipidspace/v1/pca` | Run a PCA over the submitted lipidomes |
| `GET` | `/lipidspace/v1/docs` | OpenAPI documentation |
| `GET` | `/actuator/health` | Health check — `{"status":"UP"}` plus disk details |

Example request:

```bash
curl -X POST -H 'Content-Type: application/json' \
     --data-binary "@examples/Rest/Plasma-Singapore-Short.json" \
     localhost:8888/lipidspace/v1/pca
```

<details><summary>Expected response</summary>
<p>

```
  {"LipidSpaces": [{"LipidomeName": "global_lipidome", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [1, 1, 1, 1, 1, 1], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11081707", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [325, 438, 1905, 538, 147, 136], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11062901", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [263, 367, 2253, 555, 167, 159], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11050509", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [237, 262, 1786, 444, 183, 139], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11060206", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [134, 230, 1736, 297, 97, 93], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}], "LipidomeDistanceMatrix": [[0, 1.13403, 0.573534, 0.785351], [1.13403, 0, 1.52182, 1.68476], [0.573534, 1.52182, 0, 0.479032], [0.785351, 1.68476, 0.479032, 0]]} 
```
</p>
</details>

## Docker

The image builds LipidSpaceRest from source on `ubuntu:24.04` and ships only what the service needs at runtime.

### Building

```bash
docker build -f LipidSpaceRest.docker \
    --build-arg RELEASE_VERSION="$(git describe --tags --always --dirty)" \
    -t docker.lifs-tools.org/lipidspace:latest .
```

`RELEASE_VERSION` matters: git is not installed in the build stage, so without it the binary falls back to `LIPIDSPACE_FALLBACK_VERSION` and the image reports a version that has nothing to do with its contents. The release workflow passes the git tag here. Confirm what you got:

```bash
docker run --rm docker.lifs-tools.org/lipidspace:latest --version
```

### Running

```bash
docker run -p 8888:8888 --rm docker.lifs-tools.org/lipidspace:latest
```

The entry point is `./LipidSpaceRest --tmp_folder /tmp`; anything you append becomes an additional option:

```bash
docker run -p 9000:9000 --rm docker.lifs-tools.org/lipidspace:latest \
    --port 9000 --disk-threshold-mb 50 --debug
```

The container writes per-request scratch data under `/tmp`, which the vacuum thread sweeps on the schedule described above. Mount a volume there if you want that traffic off the container's writable layer:

```bash
docker run -p 8888:8888 --rm -v lipidspace-tmp:/tmp \
    docker.lifs-tools.org/lipidspace:latest
```

### Health check

`/actuator/health` reports `DOWN` when free space on the temp folder falls below the threshold, which makes it usable as a container health check:

```bash
curl -s localhost:8888/actuator/health
```

```json
{"status":"UP","components":{"diskSpace":{"status":"UP","details":{"total":...,"free":...,"threshold":10485760,"path":"/tmp"}}}}
```

```yaml
healthcheck:
  test: ["CMD", "curl", "-f", "http://localhost:8888/actuator/health"]
  interval: 30s
  timeout: 5s
  retries: 3
```

The `curl` example from the previous section works unchanged against the container.
