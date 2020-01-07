# C++ implementation for Goslin

This is the Goslin reference implementation for C++.

> **_NOTE:_**  This is an *early* development version, please use at your own risk and report issues to help improve it!

The cppgoslin has been developed with regard the following general issues:

1. Ease the handling with lipid names for developers working on mass spectrometry-based lipidomics tools.
2. Offering a tool to unify all existing dialects of lipid names.


## Installation

### Prerequisites
The cppgoslin library needs a g++ compiler version capable of *C++ 11* standard and has simple makefiles for easy compilation and installation. 

```
g++
make
```

To install the library globally on your system, simply type:

```
[sudo] make install
```

Be sure that you have root permissions. Here, the library and headers are installed into the /usr directory. If you want to change, you have to edit the firstline in the **makefile**.


### Testing cppgoslin

To run the tests, please type:

```
make test
make runtests
```

If a test should fail, please contact the developers.


### Using cppgoslin

The two major functions within cppgoslin are the parsing and printing of lipid names. A minimalistic example will demonstrate both functions the easiest way. In the *examples* folder, you will find the *lipid_name_parser.cpp* file. Compile it by using


```
cd examples
make
./lipid_name_parser
```

within the folder. Be aware that when changing the installing directory you also have to change the library directory within the examples *makefile*.
