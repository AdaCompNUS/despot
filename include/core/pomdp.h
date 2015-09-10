#ifndef POMDP_H
#define POMDP_H

#include "globals.h"
#include "belief.h"
#include "random_streams.h"
#include "history.h"
#include "lower_bound.h"
#include "policy.h"
#include "upper_bound.h"
#include "util/memorypool.h"
#include "util/seeds.h"
#include "util/util.h"

using namespace Globals;

/* =============================================================================
 * State class
 * =============================================================================*/
/**
 * Base state class.
 */
class State: public MemoryObject {
public:
	int state_id;
	int scenario_id;
	double weight;

	State();
	State(int _state_id, double weight);
	virtual ~State();

	friend ostream& operator<<(ostream& os, const State& state);

	virtual string text() const;

	static double Weight(const vector<State*>& particles);

	State* operator()(int state_id, double weight) {
		this->state_id = state_id;
		this->weight = weight;
		return this;
	}
};

/* =============================================================================
 * StateIndexer class
 * =============================================================================*/
/**
 * Interface for a mapping between states and indices.
 */
class StateIndexer {
public:
	virtual ~StateIndexer();

	virtual int NumStates() const = 0;
	virtual int GetIndex(const State* state) const = 0;
	virtual const State* GetState(int index) const = 0;
};

/* =============================================================================
 * StatePolicy class
 * =============================================================================*/
/**
 * Interface for a mapping from states to actions.
 */
class StatePolicy {
public:
	virtual ~StatePolicy();
	virtual int GetAction(const State& state) const = 0;
};

/* =============================================================================
 * MMAPinferencer class
 * =============================================================================*/
/**
 * Interface for computing marginal MAP state from a set of particles.
 */
class MMAPInferencer {
public:
	virtual ~MMAPInferencer();

	virtual const State* GetMMAP(const vector<State*>& particles) const = 0;
};

class POMCPPrior;

/* =============================================================================
 * DSPOMDP class
 * =============================================================================*/
/**
 * Interface for a deterministic simulative model for POMDP.
 */
class DSPOMDP {
public:
	DSPOMDP();

	virtual ~DSPOMDP();

	/* ========================================================================
	 * Deterministic simulative model and related functions
	 * ========================================================================*/
	/**
	 * Determistic simulative model for POMDP.
	 */
	virtual bool Step(State& state, double random_num, int action,
		double& reward, OBS_TYPE& obs) const = 0;

	/**
	 * Override this to get speedup for LookaheadUpperBound.
	 */
	virtual bool Step(State& state, double random_num, int action,
		double& reward) const;

	/**
	 * Simulative model for POMDP.
	 */
	virtual bool Step(State& state, int action, double& reward,
		OBS_TYPE& obs) const;

	/* ========================================================================
	 * Action
	 * ========================================================================*/
	/**
	 * Returns number of actions.
	 */
	virtual int NumActions() const = 0;

	/* ========================================================================
	 * Functions related to beliefs and starting states.
	 * ========================================================================*/
	/**
	 * Returns the observation probability.
	 */
	virtual double ObsProb(OBS_TYPE obs, const State& state,
		int action) const = 0;

	/**
	 * Returns a starting state.
	 */
	virtual State* CreateStartState(string type = "DEFAULT") const = 0;

	/**
	 * Returns the initial belief.
	 */
	virtual Belief* InitialBelief(const State* start,
		string type = "DEFAULT") const = 0;

	/* ========================================================================
	 * Bound-related functions.
	 * ========================================================================*/
	/**
	 * Returns the maximum reward.
	 */
	virtual double GetMaxReward() const = 0;
	virtual ParticleUpperBound* CreateParticleUpperBound(string name = "DEFAULT") const;
	virtual ScenarioUpperBound* CreateScenarioUpperBound(string name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	/**
	 * Returns (a, v), where a is an action with largest minimum reward when it is
	 * executed, and v is its minimum reward, that is, a = \max_{a'} \min_{s}
	 * R(a', s), and v = \min_{s} R(a, s).
	 */
	virtual ValuedAction GetMinRewardAction() const = 0;
	virtual ParticleLowerBound* CreateParticleLowerBound(string name = "DEFAULT") const;
	virtual ScenarioLowerBound* CreateScenarioLowerBound(string bound_name = "DEFAULT",
		string particle_bound_name = "DEFAULT") const;

	virtual POMCPPrior* CreatePOMCPPrior(string name = "DEFAULT") const;

	/* ========================================================================
	 * Display
	 * ========================================================================*/
	/**
	 * Prints a state.
	 */
	virtual void PrintState(const State& state, ostream& out = cout) const = 0;

	/**
	 * Prints an observation.
	 */
	virtual void PrintObs(const State& state, OBS_TYPE obs,
		ostream& out = cout) const = 0;

	/**
	 * Prints an action.
	 */
	virtual void PrintAction(int action, ostream& out = cout) const = 0;

	/**
	 * Prints a belief.
	 */
	virtual void PrintBelief(const Belief& belief,
		ostream& out = cout) const = 0;

	/* ========================================================================
	 * Memory management.
	 * ========================================================================*/
	/**
	 * Allocate a state.
	 */
	virtual State* Allocate(int state_id = -1, double weight = 0) const = 0;

	/**
	 * Returns a copy of the state.
	 */
	virtual State* Copy(const State* state) const = 0;

	/**
	 * Returns a copy of the particle.
	 */
	virtual void Free(State* state) const = 0;

	/**
	 * Returns a copy of the particles.
	 */
	vector<State*> Copy(const vector<State*>& particles) const;

	/**
	 * Returns number of allocated particles.
	 */
	virtual int NumActiveParticles() const = 0;

	/**
	 * Returns a copy of this model.
	 */
	inline virtual DSPOMDP* MakeCopy() const {
		return NULL;
	}
};

/* =============================================================================
 * BeliefMDP class
 * =============================================================================*/
/**
 * The BeliefMDP class provides an interface for the belief MDP, which is
 * commonly used in belief tree search algorithms.
 *
 * @see AEMS
 */

class BeliefMDP: public DSPOMDP {
public:
	BeliefMDP();
	virtual ~BeliefMDP();

	virtual BeliefLowerBound* CreateBeliefLowerBound(string name) const;
	virtual BeliefUpperBound* CreateBeliefUpperBound(string name) const;

  /**
   * Transition function for the belief MDP.
   */
	virtual Belief* Tau(const Belief* belief, int action,
		OBS_TYPE obs) const = 0;

  /**
   * Observation function for the belief MDP.
   */
	virtual void Observe(const Belief* belief, int action,
		map<OBS_TYPE, double>& obss) const = 0;

  /**
   * Reward function for the belief MDP.
   */
	virtual double StepReward(const Belief* belief, int action) const = 0;
};

#endif
