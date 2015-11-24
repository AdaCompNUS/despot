#ifndef POMDP_H
#define POMDP_H

#include <despot/core/globals.h>
#include <despot/core/belief.h>
#include <despot/random_streams.h>
#include <despot/core/history.h>
#include <despot/core/lower_bound.h>
#include <despot/core/policy.h>
#include <despot/core/upper_bound.h>
#include <despot/util/memorypool.h>
#include <despot/util/seeds.h>
#include <despot/util/util.h>

namespace despot {

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

	friend std::ostream& operator<<(std::ostream& os, const State& state);

	virtual std::string text() const;

	static double Weight(const std::vector<State*>& particles);

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

	virtual const State* GetMMAP(const std::vector<State*>& particles) const = 0;
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
	virtual State* CreateStartState(std::string type = "DEFAULT") const = 0;

	/**
	 * Returns the initial belief.
	 */
	virtual Belief* InitialBelief(const State* start,
		std::string type = "DEFAULT") const = 0;

	/* ========================================================================
	 * Bound-related functions.
	 * ========================================================================*/
	/**
	 * Returns the maximum reward.
	 */
	virtual double GetMaxReward() const = 0;
	virtual ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;
	virtual ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	/**
	 * Returns (a, v), where a is an action with largest minimum reward when it is
	 * executed, and v is its minimum reward, that is, a = \max_{a'} \min_{s}
	 * R(a', s), and v = \min_{s} R(a, s).
	 */
	virtual ValuedAction GetMinRewardAction() const = 0;
	virtual ParticleLowerBound* CreateParticleLowerBound(std::string name = "DEFAULT") const;
	virtual ScenarioLowerBound* CreateScenarioLowerBound(std::string bound_name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	virtual POMCPPrior* CreatePOMCPPrior(std::string name = "DEFAULT") const;

	/* ========================================================================
	 * Display
	 * ========================================================================*/
	/**
	 * Prints a state.
	 */
	virtual void PrintState(const State& state, std::ostream& out = std::cout) const = 0;

	/**
	 * Prints an observation.
	 */
	virtual void PrintObs(const State& state, OBS_TYPE obs,
		std::ostream& out = std::cout) const = 0;

	/**
	 * Prints an action.
	 */
	virtual void PrintAction(int action, std::ostream& out = std::cout) const = 0;

	/**
	 * Prints a belief.
	 */
	virtual void PrintBelief(const Belief& belief,
		std::ostream& out = std::cout) const = 0;

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
	std::vector<State*> Copy(const std::vector<State*>& particles) const;

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

	virtual BeliefLowerBound* CreateBeliefLowerBound(std::string name) const;
	virtual BeliefUpperBound* CreateBeliefUpperBound(std::string name) const;

  /**
   * Transition function for the belief MDP.
   */
	virtual Belief* Tau(const Belief* belief, int action,
		OBS_TYPE obs) const = 0;

  /**
   * Observation function for the belief MDP.
   */
	virtual void Observe(const Belief* belief, int action,
		std::map<OBS_TYPE, double>& obss) const = 0;

  /**
   * Reward function for the belief MDP.
   */
	virtual double StepReward(const Belief* belief, int action) const = 0;
};

} // namespace despot

#endif
