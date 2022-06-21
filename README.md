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

  curl -v -H 'Content-Type: application/json' -d "{
    \"lipidomes\": [
      {\"name\":\"a\"},
      {\"name\":\"b\"}
    ]
  }" localhost:8888/lipidspace/v1/pca

## Building LipidSpace REST Docker

  docker build -f LipidSpaceRest.docker -t lifs-tools.org/lipidspace:latest .

  docker run -p8888:8888 --rm lifs-tools.org/lipidspace:latest

The same curl call from above works on the running Docker container.

# Defining Git Submodules for libraries

cpp-httplib

  git submodule add git@github.com:yhirose/cpp-httplib.git cpp-httplib
  git checkout <TAG_VERSION>
