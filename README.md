# DESPOT

## Overview
Approximate POMDP Planning Online (APPL Online) Toolkit

This software package is a C++ implementation of the DESPOT algorithm<sup>1</sup>.

[1] [**DESPOT: Online POMDP Planning with Regularization**](https://www.jair.org/media/5328/live-5328-9753-jair.ps). *Nan Ye, Adhiraj Somani, David Hsu and Wee Sun Lee*. 
This implementation extends our [NIPS 2013 paper](http://bigbird.comp.nus.edu.sg/pmwiki/farm/motion/uploads/Site/nips13.pdf) with an improved search algorithm, analysis and more empirical results.

[Copyright &copy; 2014-2017 by National University of Singapore](http://motion.comp.nus.edu.sg/).

## Table of Contents

* [Requirements](#requirements)
* [Installation](#installation)
* [Quick Start](#quick-start)
* [Integration](#integration)
* [Documentation](#documentation)
* [Package Contents](#package-contents)
* [Acknowledgements](#acknowledgements)
* [Bugs and Suggestions](#bugs-and-suggestions)
* [Release Notes](#release-notes)

## Requirements

Tested Operating Systems:

<!--| Linux 14.04| OS X (10.1)  | Windows  |
|:------------- |:-------------:|: -----:|
|[![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges)| [![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges) | Not Supported |-->

| Linux       | OS X
| :-------------: |:-------------:|
|[![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges)      | [![Build Status](https://semaphoreapp.com/api/v1/projects/d4cca506-99be-44d2-b19e-176f36ec8cf1/128505/shields_badge.svg)](https://semaphoreapp.com/boennemann/badges) 

Tested Compilers: gcc | g++ 4.2.1 or above

Tested Hardware: Intel Core i7 CPU, 2.0 GB RAM

Other Dependencies: (Optional) [CMake (2.8+)](https://cmake.org/install/)

## Installation

Clone OR [Download and Unzip](https://github.com/AdaCompNUS/despot/archive/master.zip):
```bash
$ git clone https://github.com/AdaCompNUS/despot.git
```

Compile:
```bash
$ cd despot
$ make
```

This will generate libraries and header files inside a local folder in `despot`. To install them into your system directory, see [Integration](#integration).

## Quick Start

DESPOT can be used to solve a POMDP specified in the **POMDPX** format or a POMDP
specified in **C++** according to the API. We illustrate this on the [Tiger](http://people.csail.mit.edu/lpk/papers/aij98-pomdp.pdf) problem.

1.To run Tiger specified in [C++](doc/cpp_model_doc), compile and run: 
```bash
$ cd despot/examples/cpp_models/tiger
$ make
$ ./tiger --runs 2
```

This command computes and simulates DESPOT's policy for `N = 2` runs and reports the
performance for the tiger problem specified in C++. See [doc/Usage.txt](doc/Usage.txt) for more options.

2.To run Tiger specified in [POMDPX format](http://bigbird.comp.nus.edu.sg/pmwiki/farm/appl/index.php?n=Main.PomdpXDocumentation.), compile and run:

```bash
$ cd despot/examples/pomdpx_models
$ make
$ ./pomdpx -m ./data/Tiger.pomdpx --runs 2 
```

This command computes and simulates DESPOT's policy for `N = 2` runs and reports the
performance for the tiger problem specified in POMDPX format. See [doc/Usage.txt](doc/Usage.txt) for 
more options.


## Integration

If you are interested in integrating DESPOT into an existing project or using an IDE for editing, we provide a [CMakeLists.txt](CMakeLists.txt).

To install DESPOT libraries and header files into your system directory:
```bash
$ cd despot
$ mkdir build; cd build
$ cmake ../
$ make
$ sudo make install
```

To integrate DESPOT into your project, add this to your `CMakeLists.txt` file:

```CMake
find_package(Despot CONFIG REQUIRED)

add_executable("YOUR_PROJECT_NAME"
  <your_src_files>
)

target_link_libraries("YOUR_PROJECT_NAME"
  despot
)
```

## Documentation

Documentation can be found in the "[doc](doc/)" directory. 

For a description of our example domains and more POMDP problems see [the POMDP page](http://www.pomdp.org/examples/).


## Package Contents

```
Makefile                  Makefile for compiling the solver library
README.md                 Overview
include                   Header files
src/core                  Core data structures for the solvers
src/solvers               Solvers, including despot, pomcp and aems
src/pomdpx                Pomdpx and its parser
src/util                  Math and logging utilities
src/ippc                  Interface for International Probabilistic Planning Competition
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
Silver's [POMCP code](http://www0.cs.ucl.ac.uk/staff/D.Silver/web/Applications.html)

## Bugs and Suggestions
Please use the issue tracker.

## Release Notes
2015/09/28 Initial release.

2017/03/07 Public release. Revised documentation.

