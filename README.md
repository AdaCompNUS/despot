#DESPOT

## Overview
Approximate POMDP Planning Online (APPL Online) Toolkit

This software package is a C++ implementation of the DESPOT algorithm <sup>1</sup>.

[1] [**Online POMDP Planning with Regularization**](http://bigbird.comp.nus.edu.sg/pmwiki/farm/motion/uploads/Site/nips13.pdf). Nan Ye, Adhiraj Somani, David Hsu and Wee Sun Lee. 
This extends our NIPS 2013 paper with an improved search algorithm, its analysis, and more empirical results.

Copyright &copy; 2014-2015 by National University of Singapore.

## Requirements

Operating systems:

<!--| Linux (14.04)| OS X (10.1)  | Windows  |
|:------------- |:-------------:|: -----:|
|[![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges)| [![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges) | Not Supported |-->

| Linux (14.04)        | OS X (12.1)           | Windows  |
| ------------- |:-------------:| -----:|
| col 3 is      | right-aligned | $1600 |

[CMake (2.8+)](https://cmake.org/install/)

Tested Compilers: gcc/g++ 4.2.1 or above

## Installation

Clone, compile and install:
```bash
git clone https://github.com/AdaCompNUS/despot.git
cd despot

mkdir build; cd build
cmake ../
make
sudo make install
```

## Examples

DESPOT can be used to solve a POMDP specified in the POMDPX format or a POMDP
specified in C++ according to the API. We illustrate this on the Tag problem.

1.To run Tag specified in POMDPX format, compile and run:

```bash
cd <despot_dir>/examples/pomdpx_models

mkdir build; cd build
cmake ../
make

./pomdpx -m ../data/tag.pomdpx --runs <N> [OPTION]...
```

This command simulates DESPOT's policy for N times and reports the
performance for tag problem specified in POMDPX format. See doc/Usage.txt for 
more options.

2.To run Tag specified in C++, compile and run: 
```bash
cd <despot_dir>/examples/cpp_models/tag

mkdir build; cd build
cmake ../
make

./tag --runs <N> [OPTION]...
```

This command simulates DESPOT's policy for N times and reports the
performance for tag problem specified in C++. See doc/Usage.txt for more options.


## Documentation

Documentation can be found in the directory "doc". See PACKAGE CONTENTS for a detailed listing.


## Package Contents

```
Makefile                  Makefile for compiling the solver library
README.md                 Overview
include                   Header files
src                       Source files
license                   Licenses and attributions
examples/cpp_models       POMDP models implemented in C++
examples/pomdpx_models    POMDP models implemented in pomdpx
doc/pomdpx_model_doc      Documentation for POMDPX file format
doc/cpp_model_doc         Documentation for implementing POMDP models in C++
doc/Usage.txt             Explanation of command-line options
doc/nips2013.txt          Instruction to obtain results in [1]
```

## Acknowledgements

Pocman implementation and memorypool.h in the package are based on David
Silver's POMCP code, which is available at

  http://www0.cs.ucl.ac.uk/staff/D.Silver/web/Applications.html

## Bugs and Suggestions
Please use the issue tracker.

## Release Notes
2015/09/28 Initial release.

