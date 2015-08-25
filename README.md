Approximate POMDP Planning Online (APPL Online) Toolkit

Copyright (c) 2014-2014 by National University of Singapore.

This software package is a C++ implementation of the DESPOT algorithm [1] for online POMDP planning.

For bug reports and suggestions, please email motion@comp.nus.edu.sg.

[1] Online POMDP Planning with Regularization.

================================================================================
TABLE OF CONTENTS
================================================================================
* Requirements
* Installation
* Quick start
* Implemented C++ Models
* Command line options
* Acknowledgement
* Release notes

================================================================================
REQUIREMENTS
================================================================================

Operating systems: Linux, Mac OS X

Tested Compilers:         gcc/g++ 4.2.1 or above

* General

  GNU make is required for building.

  The code uses features of the C++11 standard.

================================================================================
QUICK START
================================================================================

Go to the package's root directory, type 

	make

it creates a binary executable `bin/despot_pomdpx`, which reads pomdpx format file
then outputs results. And it also generates despot library `lib/despot.a`.


DESPOT can be used to solve a POMDP specified in the POMDPX format using the
following command: (Tutorial on how to implement POMDPX-format files can be found 
at `tutorial/tutorial.htm`. Some example POMDPX-format files can be found in `pomdpx_files`.)

	bin/despot_pomdpx -m <POMDPX file> --runs <N> [OPTION]...

The above command performs N simulations. Each simulation consists of
iterations of online search and action execution. The search is done with
default discount factor of 0.95 and 500 scenarios. The initial state is
randomly drawn from the initial belief, and the simulation is terminated when
a terminal state is encountered or after a default of 90 steps. There are
various command line options that can be used. See command line options for
more details.

By default, the software prints out some useful information on the simulations
for the user. For example, the software prints out the initial world state,
and at each step, it also prints out the action, current world state,
observation, observation probability, one step reward, current total
discounted reward and current total undiscounted reward.  These outputs can
be silenced by using the command line option --silence.  When coding a
model, these outputs are pretty useful for checking correctness.  In
addition, more outputs can be generated using the --verbosity or -v option.
There are 6 different levels of verbosity: NONE, WARN, ERROR, INFO, DEBUG,
VERBOSE, which correspond to values from 0 to 5 respectively. The default
verbosity level is 0. For example, you can use `-v 2` option to set the 
verbosity level to ERROR; you can use `-v 5` option to set the verbosity level 
to VERBOSE. Refer to the COMMAND LINE OPTIONS section to see how other options are set.

DESPOT can also be used to solve a POMDP specified using C++. You can find tutorials 
on how to implement the C++ model from the file `tutorial/tutorial.htm`. The model is
required to implemet the DSPOMDP interface in the package. The package comes
with C++ models for several benchmark problems, and they can be solved using
commands similar to the one for a POMDPX model. There are several example models
implemented for you in `example_models`. For example, to generate program 
solving Tag, go to `example_models/tag`, and run the makefile in the directory:

	make

To solve tag, run the following command:

	bin/tag --runs <N> [OPTION]...

================================================================================
IMPLEMENTED C++ MODELS
================================================================================
C++ models have been included in the package for the following problems:
adventurer, bridge, tag, lasertag, rocksample, pocman. Generate the program in the 
respective directory.

The following commands can be used to obtain results in [1]:

	bin/adventurer --runs 2000 

	bin/adventurer -p 10 --runs 2000

	bin/bridge --runs 2000

	bin/tag --runs 2000

	bin/lasertag --runs 2000

	bin/rocksample --size 7 --number 8 --runs 2000

	bin/rocksample --size 11 --number 11 --runs 2000

	bin/rocksample --size 15 --number 15 --runs 2000

	bin/pocman --runs 2000

Note that doing 2000 runs can take a long time. It is better to do fewer runs
using multiple commands, i.e., run multiple commands in parallel. For example, we 
can use 20 commands like the following to do 2000 runs:
  
	bin/tag --runs 100 -r <seed>

Use a random number for the seed used in each command.

Note that there are some parameters important for improving the performance, such as 
lower/upper bound type, number of particles, etc. See the following command line options 
to learn to set those parameters.

================================================================================
COMMAND LINE OPTIONS
================================================================================
	--help                     Print usage and exit.

	-m <arg>  --model-params <arg>       Path to model-parameters file, if any.

	-d <arg>  --depth <arg>              Maximum depth of search tree (default 90).

	-g <arg>  --discount <arg>           Discount factor (default 0.95).

	--size <arg>               Size of a problem (problem specific).

	--number <arg>             Number of elements of a problem (problem specific).

	-r <arg>  --seed <arg>               Random number seed (default is random).

	-t <arg>  --timeout <arg>            Search time per move, in seconds (default 1).

	-n <arg>  --nparticles <arg>         Number of particles (default 500).

	-p <arg>  --prune <arg>              Pruning constant (default no pruning).

	--xi <arg>                 Gap constant (default to 0.95).

	-s <arg>  --simlen <arg>             Number of steps to simulate. (default 90; 0 = infinite).

	--simulator <arg>          Use IPPC server or a POMDP model as the simulator.

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

	--noise <arg>              Noise level for transition in POMDPX belief update.

================================================================================
ACKNOWLEDGEMENTS
================================================================================
Pocman implementation and memorypool.h in the package are based on David
Silver's POMCP code, which is available at
  http://www0.cs.ucl.ac.uk/staff/D.Silver/web/Applications.html

================================================================================
RELEASE NOTES
================================================================================
* 2014/xx/xx Initial release.

[1] Somani A, Ye N, Hsu D, et al. Despot: Online pomdp planning with regularization[C], Advances In Neural Information Processing Systems. 2013: 1772-1780.
