#ifndef POMDP_H
#define POMDP_H

#include <despot/core/globals.h>
#include <despot/interface/belief.h>
#include <despot/random_streams.h>
#include <despot/core/history.h>
#include <despot/interface/default_policy.h>
#include <despot/interface/lower_bound.h>
#include <despot/interface/upper_bound.h>
#include <despot/util/memorypool.h>
#include <despot/util/seeds.h>
#include <despot/util/util.h>

namespace despot {

/* =============================================================================
 * State class
 * =============================================================================*/
/**
 * [Optional interface] Base state class. (define your custom state by inheriting this class)
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
 * [Optional interface] Interface for a mapping between states and indices.
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
 * [Optional interface] Interface for a mapping from states to actions.
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
 * [Optional interface] Interface for computing marginal MAP state from a set of particles.
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
	 * [Essential interface] Determistic simulative model for POMDP.
	 */
	virtual bool Step(State& state, double random_num, ACT_TYPE action,
		double& reward, OBS_TYPE& obs) const = 0;

	/**
	 * [Optional interface] Override this to get speedup for LookaheadUpperBound.
	 */
	virtual bool Step(State& state, double random_num, ACT_TYPE action,
		double& reward) const;

	/**
	 * [Optional interface] Simulative model for POMDP.
	 */
	virtual bool Step(State& state, ACT_TYPE action, double& reward,
		OBS_TYPE& obs) const;

	/* ========================================================================
	 * Action
	 * ========================================================================*/
	/**
	 * [Essential interface] Returns number of actions.
	 */
	virtual int NumActions() const = 0;

	/* ========================================================================
	 * Reward
	 * ========================================================================*/
	/**
	 * [Optional interface] Returns the reward for taking an action at a state ( to help evaluate the real planning process)
	 */
	virtual double Reward(const State& state, ACT_TYPE action) const;

	/* ========================================================================
	 * Functions related to beliefs and starting states.
	 * ========================================================================*/
	/**
	 * [Essential interface] Returns the observation probability.
	 */
	virtual double ObsProb(OBS_TYPE obs, const State& state,
		ACT_TYPE action) const = 0;

	/**
	 * [Optional interface] Returns a starting state of simulation.
	 */
	virtual State* CreateStartState(std::string type = "DEFAULT") const;

	/**
	 * [Essential interface] Returns the initial belief.
	 */
	virtual Belief* InitialBelief(const State* start,
		std::string type = "DEFAULT") const = 0;

	/* ========================================================================
	 * Bound-related functions.
	 * ========================================================================*/
	/**
	 * [Essential interface] Returns the maximum reward.
	 */
	virtual double GetMaxReward() const = 0;

	/**
	 * [Optional interface] Override to create custom ParticleUpperBounds for solvers
	 */
	virtual ParticleUpperBound* CreateParticleUpperBound(std::string name = "DEFAULT") const;

	/**
	 * [Optional interface] Override to create custom ScenarioUpperBounds for solvers
	 */
	virtual ScenarioUpperBound* CreateScenarioUpperBound(std::string name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	/**
	 * [Essential interface] Returns (a, v), where a is an action with largest minimum reward when it is
	 * executed, and v is its minimum reward, that is, a = \max_{a'} \min_{s}
	 * R(a', s), and v = \min_{s} R(a, s).
	 */
	virtual ValuedAction GetBestAction() const = 0;

	/**
	 * [Optional interface] Override to create custom ParticleLowerBounds for solvers
	 */
	virtual ParticleLowerBound* CreateParticleLowerBound(std::string name = "DEFAULT") const;

	/**
	 * [Optional interface] Override to create custom ScenarioLowerBounds for solvers
	 */
	virtual ScenarioLowerBound* CreateScenarioLowerBound(std::string bound_name = "DEFAULT",
		std::string particle_bound_name = "DEFAULT") const;

	/**
	 * [Optional interface] Override to create custom priors for the POMCP solver
	 */
	virtual POMCPPrior* CreatePOMCPPrior(std::string name = "DEFAULT") const;

	/* ========================================================================
	 * Display
	 * ========================================================================*/
	/**
	 * [Essential interface] Prints a state.
	 */
	virtual void PrintState(const State& state, std::ostream& out = std::cout) const = 0;

	/**
	 * [Essential interface] Prints an observation.
	 */
	virtual void PrintObs(const State& state, OBS_TYPE obs,
		std::ostream& out = std::cout) const = 0;

	/**
	 * [Essential interface] Prints an action.
	 */
	virtual void PrintAction(ACT_TYPE action, std::ostream& out = std::cout) const = 0;

	/**
	 * [Essential interface] Prints a belief.
	 */
	virtual void PrintBelief(const Belief& belief,
		std::ostream& out = std::cout) const = 0;

	/* ========================================================================
	 * Memory management.
	 * ========================================================================*/
	/**
	 * [Essential interface] Allocate a state.
	 */
	virtual State* Allocate(int state_id = -1, double weight = 0) const = 0;

	/**
	 * [Essential interface] Returns a copy of the state.
	 */
	virtual State* Copy(const State* state) const = 0;

	/**
	 * [Essential interface] Returns a copy of the particle.
	 */
	virtual void Free(State* state) const = 0;

	/**
	 * [Optional interface] Returns a copy of the particles.
	 */
	std::vector<State*> Copy(const std::vector<State*>& particles) const;

	/**
	 * [Essential interface] Returns number of allocated particles.
	 */
	virtual int NumActiveParticles() const = 0;

	/**
	 * [Optional interface] Returns a copy of this model.
	 */
	inline virtual DSPOMDP* MakeCopy() const {
		return NULL;
	}
};

/* =============================================================================
 * BeliefMDP class
 * =============================================================================*/
/**
 * [Optional interface] The BeliefMDP class provides an interface for the belief MDP, which is
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
	virtual Belief* Tau(const Belief* belief, ACT_TYPE action,
		OBS_TYPE obs) const = 0;

  /**
   * Observation function for the belief MDP.
   */
	virtual void Observe(const Belief* belief, ACT_TYPE action,
		std::map<OBS_TYPE, double>& obss) const = 0;

  /**
   * Reward function for the belief MDP.
   */
	virtual double StepReward(const Belief* belief, ACT_TYPE action) const = 0;
};

} // namespace despot

#endif
