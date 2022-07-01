# lipidspace
LipidSpace

## Prerequisites

### Cloning the repository

  git clone --recurse-submodules git@github.com:lifs-tools/lipidspace.git

To initialize the library submodules manually, e.g. if you cloned without `--recurse-submodules` run:

  git submodule update --init --recursive

### Linux

qt5
qmake
make

See the `LipidSpaceRest.docker` file for required Ubuntu packages.

## Building LipidSpace

  qmake LipidSpace.pro
  make

## Building LipidSpace REST

  qmake LipidSpaceRest.pro
  make

### Running the REST server

LipidSpaceRest

Using curl to test the server:

  curl -X POST -v -H 'Content-Type: application/json' --data-binary "@examples/Rest/Plasma-Singapore-Short.json" localhost:8888/lipidspace/v1/pca

Which should return:

  {"LipidSpaces": [{"LipidomeName": "global_lipidome", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [1, 1, 1, 1, 1, 1], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11081707", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [325, 438, 1905, 538, 147, 136], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11062901", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [263, 367, 2253, 555, 167, 159], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11050509", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [237, 262, 1786, 444, 183, 139], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}, {"LipidomeName": "M11060206", "LipidNames": ["Cer 18:0;(OH)2/22:0", "Cer 18:0;(OH)2/24:0", "Cer 18:0;O2/24:1", "Cer 18:1;O2/16:0", "Cer 18:1;O2/18:0", "Cer 18:1;O2/20:0"], "Intensities": [134, 230, 1736, 297, 97, 93], "X": [-1.35718, -2.11602, -2.17246, 3.14558, 2.19029, 0.309797], "Y": [0.82256, -0.697639, -1.08739, -1.41868, 0.533233, 1.84791]}], "LipidomeDistanceMatrix": [[0, 1.13403, 0.573534, 0.785351], [1.13403, 0, 1.52182, 1.68476], [0.573534, 1.52182, 0, 0.479032], [0.785351, 1.68476, 0.479032, 0]]} 

## Building LipidSpace REST Docker

  docker build -f LipidSpaceRest.docker -t lifs-tools.org/lipidspace:latest .

  docker run -p8888:8888 --rm lifs-tools.org/lipidspace:latest

The same curl call from above works on the running Docker container.

# Defining Git Submodules for libraries

cpp-httplib

  git submodule add git@github.com:yhirose/cpp-httplib.git cpp-httplib
  git checkout <TAG_VERSION>
