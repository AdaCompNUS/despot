#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "globals.h"
#include "pomdp.h"
#include "pomdpx/pomdpx.h"
#include "client.h"
#include "util/util.h"

/* =============================================================================
 * IPPCLog class
 * =============================================================================*/

class IPPCLog {
private:
	vector<string> runned_instances;
	vector<int> num_of_completed_runs;
	string log_file_;

public:
	static time_t start_time;

	static double curr_inst_start_time;
	static double curr_inst_target_time; // Targetted amount of time used for each step
	static double curr_inst_budget; // Total time in seconds given for current instance
	static double curr_inst_remaining_budget; // Remaining time in seconds for current instance
	static int curr_inst_steps;
	static int curr_inst_remaining_steps;
	static double allocated_time;
	static double plan_time_ratio;

	IPPCLog(string log_file);

	void Save();
	void IncNumOfCompletedRuns(string problem);
	int GetNumCompletedRuns() const;
	int GetNumRemainingRuns() const;
	int GetNumCompletedRuns(string instance) const;
	int GetNumRemainingRuns(string instance) const;
	double GetUsedTimeInSeconds() const;
	double GetRemainingTimeInSeconds() const;

	// Pre-condition: curr_inst_start_time is initialized
	void SetInitialBudget(string instance);
	double GetRemainingBudget(string instance) const;
};

/* =============================================================================
 * Simulator class
 * =============================================================================*/

/** Interface for performing simulations to evaluate a solver on a problem.*/
class Simulator {
protected:
	DSPOMDP* model_;
	string belief_type_;
	Solver* solver_;
	clock_t start_clockt_;
	State* state_;
	int step_;
	double target_finish_time_;
	ostream* out_;

	vector<double> discounted_round_rewards_;
	vector<double> undiscounted_round_rewards_;
	double reward_;
	double total_discounted_reward_;
	double total_undiscounted_reward_;

public:
	Simulator(DSPOMDP* model, string belief_type, Solver* solver,
		clock_t start_clockt, ostream* out);
	virtual ~Simulator();

	inline void out(ostream* o) {
		out_ = o;
	}
	inline void rewards(vector<double> rewards) {
		undiscounted_round_rewards_ = rewards;
	}
	inline vector<double> rewards() {
		return undiscounted_round_rewards_;
	}

	inline int step() {
		return step_;
	}
	inline double target_finish_time() {
		return target_finish_time_;
	}
	inline void target_finish_time(double t) {
		target_finish_time_ = t;
	}
	inline Solver* solver() {
		return solver_;
	}
	inline void solver(Solver* s) {
		solver_ = s;
	}
	inline DSPOMDP* model() {
		return model_;
	}
	inline void model(DSPOMDP* m) {
		model_ = m;
	}

	virtual inline void world_seed(unsigned seed) {
	}

	virtual int Handshake(string instance) = 0; // Initialize simulator and return number of runs.
	virtual void InitRound() = 0;

	bool RunStep();

	virtual double EndRound() = 0; // Return total undiscounted reward for this round.
	virtual bool ExecuteAction(int action, double& reward, OBS_TYPE& obs) = 0;
	virtual void ReportStepReward();
	virtual double End() = 0; // Free resources and return total reward collected

	virtual void UpdateTimePerMove(double step_time) = 0;

	double AverageUndiscountedRoundReward() const;
	double StderrUndiscountedRoundReward() const;
	double AverageDiscountedRoundReward() const;
	double StderrDiscountedRoundReward() const;
};

/* =============================================================================
 * IPPCSimulator class
 * =============================================================================*/

/** Evaluation protocol used in IPPC'11 and IPPC'14. */
class IPPCSimulator: public Simulator {
private:
	POMDPX* pomdpx_;
	Client* client_;
	IPPCLog log_;
	string instance_;
	string hostname_;
	string port_;

public:
	IPPCSimulator(DSPOMDP* model, string belief_type, Solver* solver,
		clock_t start_clockt, string hostname, string port, string log,
		ostream* out);
	~IPPCSimulator();

	void port_number(string port);
	int GetNumCompletedRuns() const {
		return log_.GetNumCompletedRuns();
	}
	int GetNumCompletedRuns(string instance) const {
		return log_.GetNumCompletedRuns(instance);
	}

	int Handshake(string instance);
	void InitRound();
	double EndRound();
	bool ExecuteAction(int action, double& reward, OBS_TYPE& obs);
	// void ReportStepReward();
	double End();
	void UpdateTimeInfo(string instance);
	void UpdateTimePerMove(double step_time);
};

/* =============================================================================
 * POMDPSimulator class
 * =============================================================================*/

/** Evaluation by simulating using a DSPOMDP model.*/
class POMDPSimulator: public Simulator {
protected:
	Random random_;

public:
	POMDPSimulator(DSPOMDP* model, string belief_type, Solver* solver,
		clock_t start_clockt, ostream* out, double target_finish_time = -1,
		int num_steps = -1);
	~POMDPSimulator();

	virtual inline void world_seed(unsigned seed) {
		random_ = Random(seed);
	}

	int Handshake(string instance);
	void InitRound();
	double EndRound();
	bool ExecuteAction(int action, double& reward, OBS_TYPE& obs);
	double End();
	void UpdateTimePerMove(double step_time);
};

#endif
