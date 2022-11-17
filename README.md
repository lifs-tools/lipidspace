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
LipidSpace supports both csv and xlsx files for import. The tables can be either pivot tables (that is lipid species in columns, samples in rows or lipid species in rows and samples in columns) or flat tables. Further, lipidomics data files in <a href=https://pubs.acs.org/doi/10.1021/acs.analchem.8b04310>mzTab-M</a> format can be imported.
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

