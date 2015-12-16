#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <despot/core/globals.h>
#include <despot/core/pomdp.h>
#include <despot/pomdpx/pomdpx.h>
#include <despot/ippc/client.h>
#include <despot/util/util.h>

namespace despot {

/* =============================================================================
 * EvalLog class
 * =============================================================================*/

class EvalLog {
private:
  std::vector<std::string> runned_instances;
	std::vector<int> num_of_completed_runs;
	std::string log_file_;

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

	EvalLog(std::string log_file);

	void Save();
	void IncNumOfCompletedRuns(std::string problem);
	int GetNumCompletedRuns() const;
	int GetNumRemainingRuns() const;
	int GetNumCompletedRuns(std::string instance) const;
	int GetNumRemainingRuns(std::string instance) const;
	double GetUsedTimeInSeconds() const;
	double GetRemainingTimeInSeconds() const;

	// Pre-condition: curr_inst_start_time is initialized
	void SetInitialBudget(std::string instance);
	double GetRemainingBudget(std::string instance) const;
};

/* =============================================================================
 * Evaluator class
 * =============================================================================*/

/** Interface for evaluating a solver's performance by simulating how it runs
 * in a real.
 */
class Evaluator {
protected:
	DSPOMDP* model_;
	std::string belief_type_;
	Solver* solver_;
	clock_t start_clockt_;
	State* state_;
	int step_;
	double target_finish_time_;
	std::ostream* out_;

	std::vector<double> discounted_round_rewards_;
	std::vector<double> undiscounted_round_rewards_;
	double reward_;
	double total_discounted_reward_;
	double total_undiscounted_reward_;

public:
	Evaluator(DSPOMDP* model, std::string belief_type, Solver* solver,
		clock_t start_clockt, std::ostream* out);
	virtual ~Evaluator();

	inline void out(std::ostream* o) {
		out_ = o;
	}

	inline void rewards(std::vector<double> rewards) {
		undiscounted_round_rewards_ = rewards;
	}

	inline std::vector<double> rewards() {
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

	virtual int Handshake(std::string instance) = 0; // Initialize simulator and return number of runs.
	virtual void InitRound() = 0;

	bool RunStep(int step, int round);

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
 * IPPCEvaluator class
 * =============================================================================*/

/** Evaluation protocol used in IPPC'11 and IPPC'14. */
class IPPCEvaluator: public Evaluator {
private:
	POMDPX* pomdpx_;
	Client* client_;
	EvalLog log_;
	std::string instance_;
	std::string hostname_;
	std::string port_;

public:
	IPPCEvaluator(DSPOMDP* model, std::string belief_type, Solver* solver,
		clock_t start_clockt, std::string hostname, std::string port, std::string log,
		std::ostream* out);
	~IPPCEvaluator();

	void port_number(std::string port);
	int GetNumCompletedRuns() const {
		return log_.GetNumCompletedRuns();
	}
	int GetNumCompletedRuns(std::string instance) const {
		return log_.GetNumCompletedRuns(instance);
	}

	int Handshake(std::string instance);
	void InitRound();
	double EndRound();
	bool ExecuteAction(int action, double& reward, OBS_TYPE& obs);
	// void ReportStepReward();
	double End();
	void UpdateTimeInfo(std::string instance);
	void UpdateTimePerMove(double step_time);
};

/* =============================================================================
 * POMDPEvaluator class
 * =============================================================================*/

/** Evaluation by simulating using a DSPOMDP model.*/
class POMDPEvaluator: public Evaluator {
protected:
	Random random_;

public:
	POMDPEvaluator(DSPOMDP* model, std::string belief_type, Solver* solver,
		clock_t start_clockt, std::ostream* out, double target_finish_time = -1,
		int num_steps = -1);
	~POMDPEvaluator();

	virtual inline void world_seed(unsigned seed) {
		random_ = Random(seed);
	}

	int Handshake(std::string instance);
	void InitRound();
	double EndRound();
	bool ExecuteAction(int action, double& reward, OBS_TYPE& obs);
	double End();
	void UpdateTimePerMove(double step_time);
};

} // namespace despot

#endif
