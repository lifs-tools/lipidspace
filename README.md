# LipidSpace

[![CI Status](https://github.com/lifs-tools/lipidspace/actions/workflows/build.yml/badge.svg)](https://github.com/lifs-tools/lipidspace/actions/workflows/build.yml)

LipidSpace is a stand-alone tool to analyze and compare lipidomes by assessing their structural differences. A graph-based comparison of lipid structures allows to calculate distances between lipids and to determine similarities across lipidomes. It allows for a rapid (re)analysis of experiments, identifies lipids responsible for shaping the respective lipidome, and provides methods for quality
control.

LipidSpace has been built and tested under Windows 10 and Ubuntu 22.04 Linux. It comes with four built-in tutorials to get you started.

Please check the `LICENSE*` files for more information about individual license terms of embedded libraries.

## Prerequisites

LipidSpace uses the [QT6](https://www.qt.io/product/qt6) libraries for the graphical user interface.

## Frequently asked questions

<details><summary><b>What data formats can be imported?</b></summary>
<p>
LipidSpace supports both csv and xlsx files for import. The tabels can be either pivot tables (that is lipid species in columns, samples in rows or lipid species in rows and samples in columns) or flat tables. Further, lipidomics data files in [mzTab-M](https://pubs.acs.org/doi/10.1021/acs.analchem.8b04310) format can be imported.
</p>
</details>



### Cloning the repository

```
git clone --recurse-submodules git@github.com:lifs-tools/lipidspace.git
```

### Linux

Please use the latest QT 6 library.

On Ubuntu 22.04, you can run
  
```
sudo apt install \
  build-essential \
  libfontconfig1 \
  qt6-base-dev \
  qt6-base-dev-tools \
  libqt6svg6-dev \
  libqt6charts6-dev \
  libopenblas-base \
  libopenblas-dev \
  libomp-dev \
  mesa-common-dev \
  libglu1-mesa-dev \
  libc6 \
  libstdc++6
```

to install the necessary dependencies.

See the `LipidSpaceRest.docker` file for other required Ubuntu 22.04 packages.

## Building LipidSpace

```
qmake6 LipidSpace.pro
make
```

If you experience a build error like the following:

```
./ui_lipidspacegui.h:33:10: fatal error: lipidspace/CBTableWidget.h: Datei oder Verzeichnis nicht gefunden
33 | #include "lipidspace/CBTableWidget.h"
```

or

```
src/lipidspace.cpp:308:90: error: no matching function for call to ‘LipidAdduct::LipidAdduct(LipidAdduct*&)’
```

Please make sure that you have the latest version of cppgoslin (https://github.com/lifs-tools/cppgoslin) installed on your computer (make && sudo make install).

## Building LipidSpace REST

```
qmake6 LipidSpaceRest.pro
make
```

### Running the REST server

Start the LipidSpaceRest server from your terminal:
```
./LipidSpaceRest
```

Using curl to send an example request to the server:

```
curl -X POST -v -H 'Content-Type: application/json' --data-binary "@examples/Rest/Plasma-Singapore-Short.json" localhost:8888/lipidspace/v1/pca
```

Which should return:

```
  {"LipidSpaces": [{"LipidomeName": "global_lipidome", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [1, 1, 1, 1, 1, 1], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11081707", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [325, 438, 1905, 538, 147, 136], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11062901", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [263, 367, 2253, 555, 167, 159], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11050509", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [237, 262, 1786, 444, 183, 139], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11060206", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [134, 230, 1736, 297, 97, 93], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}], "LipidomeDistanceMatrix": [[0, 1.13403, 0.573534, 0.785351], [1.13403, 0, 1.52182, 1.68476], [0.573534, 1.52182, 0, 0.479032], [0.785351, 1.68476, 0.479032, 0]]} 
```

## Building LipidSpace REST Docker

```
docker build -f LipidSpaceRest.docker -t docker.lifs-tools.org/lipidspace:latest .
```

```
docker run -p8888:8888 --rm docker.lifs-tools.org/lipidspace:latest
```

The same curl call from above works on the running Docker container.

