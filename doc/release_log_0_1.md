# Despot Release 0.1 Change Log

## Package Contents (updated)

```
Makefile                  Makefile for compiling the solver library
README.md                 Overview
include                   Header files
src/interface             Interfaces to be implemented by users
src/core                  Core data structures for the solvers
src/solvers               Solvers, including despot, pomcp and aems
src/pomdpx                Pomdpx and its parser
src/util                  Math and logging utilities
license                   Licenses and attributions
examples/cpp_models       POMDP models implemented in C++
examples/pomdpx_models    POMDP models implemented in pomdpx
doc/pomdpx_model_doc      Documentation for POMDPX file format
doc/cpp_model_doc         Documentation for implementing POMDP models in C++
doc/usage.txt             Explanation of command-line options
doc/eclipse_guide.md      Guide for using Eclipse IDE for development
```

### Interface Folders
All header and source files related to despot interfaces such as *DSPOMDP*, *World*, *Belief*, etc. have been moved to the new "*interface*" folders under "[include/despot/](../include/despot)" and "[src/](../src)". These files include:
```
include/despot/interface/pomdp.h
include/despot/interface/world.h
include/despot/interface/belief.h
include/despot/interface/lower_bound.h
include/despot/interface/upper_bound.h
include/despot/interface/policy.h
src/interface/pomdp.cpp
src/interface/world.cpp
src/interface/belief.cpp
src/interface/lower_bound.cpp
src/interface/upper_bound.cpp
src/interface/policy.cpp
```

### Built-in Implementations of Interfaces
Built-in belief, lower bound, and upper bound types are now separated from the interface classes (such as *Belief*, *ScenarioLowerBound*, *ScenarioUpperBound*, and so on). These built-in implementations are moved to the following new files:
```
include/despot/core/pomdp_world.h
include/despot/core/particle_belief.h
include/despot/core/builtin_lower_bounds.h
include/despot/core/builtin_upper_bounds.h
include/despot/core/builtin_policy.h
src/core/pomdp_world.cpp
src/core/particle_belief.cpp
src/core/builtin_lower_bounds.cpp
src/core/builtin_upper_bounds.cpp
src/core/builtin_policy.cpp
``` 

## New Interfaces

### Class World
Usage:
``` c++
#include <despot/interface/world.h>
```
*World* is an abstract class added in release 0.1. It provides interfaces for integrating despot with real-world systems or simulators. These interfaces include:
``` c++
//Establish connection with simulator or system
virtual bool Connect()=0;
//Initialize or reset the environment (for simulators or POMDP world only), return the start state of the system if applicable
virtual State* Initialize()=0;
//Get the state of the system (only applicable for simulators or POMDP world)
virtual State* GetCurrentState() const = 0;
//Send action to be executed by the system, receive observations terminal signals from the system
virtual bool ExecuteAction(int action, OBS_TYPE& obs) =0;
```

### class SimpleTUI
Usage:
``` c++
#include <despot/simple_tui.h>
```
Users now need to implement one additional interface in *SimpleTUI*:
``` c++
virtual World* InitializeWorld(std::string& world_type, DSPOMDP *model, option::Option* options);
```
Users should create their own custom world, establish connection, and intialize its state in this function. A sample implementation should look like:
``` c++
World* InitializeWorld(std::string& world_type, DSPOMDP* model, option::Option* options){
   //Create a custom world as defined and implemented by the user
   World* custom_world=CustomWorld(world_type, model, options);
   //Establish connection with external system
   custom_world->Connect();
   //Initialize the state of the external system
   custom_world->Initialize();
}
```

### Class DSPOMDP
Usage:
``` c++
#include <despot/interface/pomdp.h>
```

#### Reward Function
The _ExecuteAction_ function in the *World* class doesn't generate rewards. A new virtual function *Reward* is added in the _DSPOMDP_ class to enable reward monitoring after executing an action:
``` c++
// Returns the reward for taking an action at a state
virtual double Reward(const State& state, int action) const;
```
When this _Reward_ function has been implemented, despot will log the step rewards automatically. Otherwise, despot will always report zero reward for non-POMDP types of world.

#### GetBestAction Function
The original pure virtual function:
``` c++
virtual ValuedAction GetMinRewardAction() const = 0;
```
has been renamed as:
``` c++
virtual ValuedAction GetBestAction() const = 0;
```

## Core Code Changes

### Class Evaluator
Usage:
``` c++
#include <despot/evaluator.h>
```
Many functionalities in the original *Evaluator* class, including communication with the world and planning pipeline control, have been shifted to *World* and *SimpleTUI*. The new *Evaluator* class only perform logging of time usage and other statistics. Check "[despot/evaluator.h](../include/despot/evaluator.h)" for more details.

### Class SimpleTUI
Usage:
``` c++
#include <despot/simple_tui.h>
```
*SimpleTUI* is now responsible for pipeline control. To achieve this, the following new member functions have been added:

``` c++
/*Perform one planning step*/
bool RunStep(int step, int round, Solver* solver, World* world, Evaluator* evaluator);
/*Perform planning for a fixed number of steps or till a terminal state is reached*/
void PlanningLoop(int round, Solver*& solver, World* world, Evaluator* evaluator);
/*The evaluation pipeline: repeat the planning pipeline for multiple rounds*/
void EvaluationLoop(DSPOMDP *model, World* world, Belief* belief, std::string belief_type, Solver *&solver, Evaluator *evaluator, option::Option *options, clock_t main_clock_start, int num_runs, int start_run);
/*Run the planning pipeline*/
int runPlanning(int argc, char* argv[]);
/*Run the evaluation pipeline*/
int runEvaluation(int argc, char* argv[]);
```
The old *run* function has been replaced by *runPlanning* and *runEvaluation*. Users can run the planning pipeline by calling *runPlanning* in the *main* function. The planning pipeline uses despot to perform online POMDP planning for a system till a fixed number of steps are finished or till a terminal state of the system has been reached. Alternatively, given a simulated world, the user can run the evaluation pipeline by calling *runEvaluation*. The evaluation pipeline will repeat the planning process for multiple times and evaluate the performance of despot according to the conducted rounds. Check "[despot/simple_tui.h](../include/despot/simple_tui.h)" for more details.

### Class POMDPWorld
Usage:
``` c++
#include <despot/core/pomdp_world.h>
```
*POMDPWorld* is an built-in implementation of *World* added in release 0.1. *POMDPWorld* represents the world as a *DSPOMDP* model. The same *DSPOMDP* model is shared by the despot solver. To use an existing *DSPOMDP* model as a POMDP-based world, reload the _InitializeWorld_ virtual function in *SimpleTUI* in the following way:
``` c++
World* InitializeWorld(std::string& world_type, DSPOMDP* model, option::Option* options){
   return InitializePOMDPWorld(world_type, model, options);
}
```
Check the cpp model examples ([examples/cpp_models/](../examples/cpp_models)) to see the usage. 



