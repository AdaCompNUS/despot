Approximate POMDP Planning Online (APPL Online) Toolkit

Copyright (c) 2014-2015 by National University of Singapore.

This software package is a C++ implementation of the DESPOT algorithm [1].

For bug reports and suggestions, please email motion@comp.nus.edu.sg.

[1] Online POMDP Planning with Regularization.


================================================================================
DIRECOTRY CONTENTS
================================================================================
```
Makefile     Makefile for compiling the solver library
README.txt   Overview
build        Compiled object files and library file
include      Header files
problems     POMDP models implemented in C++
src          Source files
pomdpx       Source files for generating pomdpx parser
tutorial     Tutorial on using the software package
````
================================================================================
REQUIREMENTS
================================================================================

Operating systems: Linux, Mac OS X

Tested Compilers:         gcc/g++ 4.2.1 or above

* General

  GNU make is required for building.


================================================================================
QUICK START
================================================================================

DESPOT can be used to solve a POMDP specified in the POMDPX format or a POMDP
specified in C++ according to the API. We illustrate this on the Tag problem.

1.Tag specified in POMDPX format

(a) Go to the package's root directory 

(b) Type 'make' in the command line 

(c) Run the following command

	bin/pomdpx -m pomdpx/data/tag.pomdpx --runs <N> [OPTION]...

2.Tag specified in C++ 

(a) Go to the problems/tag in the package's root directory 

(b) Type 'make' in the command line 

(c) Run the following command


	tag --runs <N> [OPTION]...

Both commands simulate DESPOT's policy for N times and report the
performance. See tutorial/tutorial.htm for explanation of the command. See
the command line options section in this file for details on the options.

================================================================================
IMPLEMENTED C++ MODELS
================================================================================
C++ models have been included in the package for the following problems:

adventurer, bridge, tag, lasertag, rocksample, pocman.

The following commands can be used to obtain results in [1], assuming you are
in the right subdirectory in the problems directory:

	./adventurer --runs 2000 

	./adventurer -p 10 --runs 2000

	./bridge --runs 2000

	./tag --runs 2000

	./lasertag --runs 2000

	./rocksample --size 7 --number 8 --runs 2000

	./rocksample --size 11 --number 11 --runs 2000

	./rocksample --size 15 --number 15 --runs 2000

	./pocman --runs 2000

Note that doing 2000 runs can take a long time. It is better to do fewer runs
using multiple commands, i.e., run in parallel. For example, we can use 20 
commands like the following to do 2000 runs:
  
	./tag --runs 100 -r <seed>

Use a random number for the seed used in each command.

================================================================================
COMMAND LINE OPTIONS
================================================================================
```
          --help                     Print usage and exit.
-q <arg>  --problem <arg>            Problem name.
-m <arg>  --model-params <arg>       Path to model-parameters file, if any.
-d <arg>  --depth <arg>              Maximum depth of search tree (default 90).
-g <arg>  --discount <arg>           Discount factor (default 0.95).
          --size <arg>               Size of a problem (problem specific).
          --number <arg>             Number of elements of a problem (problem
                                     specific).
-r <arg>  --seed <arg>               Random number seed (default is random).
-t <arg>  --timeout <arg>            Search time per move, in seconds (default
                                     1).
-n <arg>  --nparticles <arg>         Number of particles (default 500).
-p <arg>  --prune <arg>              Pruning constant (default no pruning).
          --xi <arg>                 Gap constant (default to 0.95).
-s <arg>  --simlen <arg>             Number of steps to simulate. (default 90; 0
                                     = infinite).
          --simulator <arg>          Use IPPC server or a POMDP model as the
                                     simulator.
          --max-policy-simlen <arg>  Number of steps to simulate the default
                                     policy. (default 90).
          --default-action <arg>     Type of default action to use. (default
                                     none).
          --runs <arg>               Number of runs. (default 1).
          --lbtype <arg>             Lower bound strategy, if applicable.
-l <arg>  --blbtype <arg>            Base lower bound, if applicable.
-u <arg>  --ubtype <arg>             Upper bound strategy, if applicable.
          --bubtype <arg>            Base upper bound, if applicable.
-b <arg>  --belief <arg>             Belief update strategy, if applicable.
-v <arg>  --verbosity <arg>          Verbosity level.
          --silence                  Reduce default output to minimal.
          --noise <arg>              Noise level for transition in POMDPX belief
                                     update.
```
================================================================================
ACKNOWLEDGEMENTS
================================================================================
Pocman implementation and memorypool.h in the package are based on David
Silver's POMCP code, which is available at

  http://www0.cs.ucl.ac.uk/staff/D.Silver/web/Applications.html

================================================================================
RELEASE NOTES
================================================================================
* 2015/xx/xx Initial release.

