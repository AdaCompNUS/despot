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
include/despot/interface/default_policy.h
src/interface/pomdp.cpp
src/interface/world.cpp
src/interface/belief.cpp
src/interface/lower_bound.cpp
src/interface/upper_bound.cpp
src/interface/default_policy.cpp
```
Note that the original *Policy* class have been renamed to *DefaultPolicy*. The filenames have also been changed accordingly.

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
virtual State* GetCurrentState() const;
//Send action to be executed by the system, receive observations terminal signals from the system
virtual bool ExecuteAction(ACT_TYPE action, OBS_TYPE& obs) =0;
```

### class PlannerBase
Usage:
``` c++
#include <despot/plannerbase.h>
```
The original class *SimpleTUI* has been moved to *PlannerBase* and its child class *Planner*. *PlannerBase* offers interfaces to initialze the planner, while the main pipelines are managed by the *Planner* class. Users now need to implement two additional interfaces in *PlannerBase*:
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
   return custom_world; 
}
```
Users also need to specify the solver type by implementing the following interface.
``` c++
/*
* Return the name of the intended solver ("DESPOT", "AEMS2", "POMCP", "DPOMCP", "PLB", "BLB")
*/
virtual std::string ChooseSolver()=0;
```
An example implementation looks like:
``` c++
std::string ChooseSolver(){
   return "DESPOT";
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
virtual double Reward(const State& state, ACT_TYPE action) const;
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

#### CreateStartState Function
*CreateStartState* is no longer an essential interface in *DSPOMDP*, because its functionality have been taken over by the *Initialize* function in the *World* class. However, users can optionally reload this function and make use of it, for instance, in the *InitialBelief* function.

### ACT_TYPE
Data type of actions are now specified by a more general *ACT_TYPE* flag. Users can define *ACT_TYPE* to be any valid data type (int, unsigned int, short, long...) in [despot/core/globals.h](../include/despot/core/globals.h). This change affects all interfaces and functions that take action parameters or return action values.

### Class ScenarioBaselineSolver and Class BeliefBaselineSolver
``` c++
#include <despot/baseline_solver.h>
```
These two classes are wrappers for using *ScenarioLowerBound* and *BeliefLowerBound* as baseline solvers. In particular, *ScenarioBaselineSolver* and *BeliefBaselineSolver* inherit *Solver* and overwrite the *search* function in it. The new *search* functions in the baseline solvers select an action using the *Value* function in the corresponding lower bound. For more details on the usage, check the *PlannerBase::InitializeSolver* function in [src/plannerbase.cpp](../src/plannerbase.cpp)

## Core Code Changes

### Class Logger
Usage:
``` c++
#include <despot/logger.h>
```
Functionalities in the original *Evaluator* class have been shifted to three classes: *Logger*, *World*, and *Planner*. The new *Logger* class performs logging of time usage and other statistics. The new *World* class handles the communication with the real-world external system. Finally, the *Planner* is responsible for pipeline control. Check "[despot/interface/world.h](../include/despot/interface/world.h), [despot/logger.h](../include/despot/logger.h), [despot/planner.h](../include/despot/planner.h)" for more details.

### Class Planner
Usage:
``` c++
/*To use the planning pipeline*/
#include <despot/planner.h>
```
*Planner* is now responsible for pipeline control. The despot package offers two types of pipelines: the planning pipeline handled by the *PlanningLoop* function, and evaluation pipeline handled by the *EvaluationLoop* function.

``` c++
Class Planner: public PlannerBase {
public:
   /*Perform one planning step*/
   bool runStep(Solver* solver, World* world, Logger* logger);
   /*Perform planning for a fixed number of steps or till a terminal state is reached*/
   void PlanningLoop(Solver*& solver, World* world, Logger* logger);
   /*Run the planning pipeline*/
   int runPlanning(int argc, char* argv[]);
   /*The evaluation pipeline: repeat the planning for multiple trials*/
   void EvaluationLoop(DSPOMDP *model, World* world, Belief* belief, std::string belief_type, Solver *&solver, Logger* logger, option::Option *options, clock_t main_clock_start, int num_runs, int start_run);
   /*Run the evaluation pipeline*/
   int runEvaluation(int argc, char* argv[]);
}
```
The old *run* function in the original *SimpleTUI* class has been replaced by *runPlanning* and *runEvaluation* in the *Planner* class. Users can launch the planning pipeline by inheriting the *Planner* classes, and calling *runPlanning* in the *main* function subsequently. The planning pipeline uses despot to perform online POMDP planning for a system till a fixed number of steps are finished or till a terminal state of the system has been reached. It can be customized through overwriting the *PlanningLoop* and *runStep* functions. Alternatively, given a simulated world, the user can run the evaluation pipeline by calling *Planner::runEvaluation*. The evaluation pipeline will repeat the planning process (as defined in *PLanningLoop*) for multiple times and evaluate the performance of despot according to the conducted trials. Check "[despot/planner.h](../include/despot/planner.h)" for implementation details.

### Class POMDPWorld
Usage:
``` c++
#include <despot/core/pomdp_world.h>
```
*POMDPWorld* is an built-in implementation of *World* added in release 0.1. *POMDPWorld* represents the world as a *DSPOMDP* model. The same *DSPOMDP* model is shared by the despot solver. To use an existing *DSPOMDP* model as a POMDP-based world, reload the _InitializeWorld_ virtual function in *PlannerBase* in the following way:
``` c++
World* InitializeWorld(std::string& world_type, DSPOMDP* model, option::Option* options){
   return InitializePOMDPWorld(world_type, model, options);
}
```
Check the cpp model examples ([examples/cpp_models/](../examples/cpp_models)) to see the usage. 

### Class Solver
``` c++
#include <despot/solver.h>
```
Function "Solver::Update" has been renamed to "Solver::BeliefUpdate".
