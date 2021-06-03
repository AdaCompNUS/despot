#include <despot/core/pomdp_world.h>
#include <despot/logger.h>

using namespace std;

namespace despot {

/* =============================================================================
 * EvalLog class
 * =============================================================================*/

time_t EvalLog::start_time = 0;
double EvalLog::curr_inst_start_time = 0;
double EvalLog::curr_inst_target_time = 0;
double EvalLog::curr_inst_budget = 0;
double EvalLog::curr_inst_remaining_budget = 0;
int EvalLog::curr_inst_steps = 0;
int EvalLog::curr_inst_remaining_steps = 0;
double EvalLog::allocated_time = 1.0;
double EvalLog::plan_time_ratio = 1.0;

EvalLog::EvalLog(string log_file) :
		log_file_(log_file) {
	ifstream fin(log_file_.c_str(), ifstream::in);
	if (!fin.good() || fin.peek() == ifstream::traits_type::eof()) {
		time(&start_time);
	} else {
		fin >> start_time;

		int num_instances;
		fin >> num_instances;
		for (int i = 0; i < num_instances; i++) {
			string name;
			int num_runs;
			fin >> name >> num_runs;
			runned_instances.push_back(name);
			num_of_completed_runs.push_back(num_runs);
		}
	}
	fin.close();
}

void EvalLog::Save() {
	ofstream fout(log_file_.c_str(), ofstream::out);
	fout << start_time << endl;
	fout << runned_instances.size() << endl;
	for (int i = 0; i < runned_instances.size(); i++)
		fout << runned_instances[i] << " " << num_of_completed_runs[i] << endl;
	fout.close();
}

void EvalLog::IncNumOfCompletedRuns(string problem) {
	bool seen = false;
	for (int i = 0; i < runned_instances.size(); i++) {
		if (runned_instances[i] == problem) {
			num_of_completed_runs[i]++;
			seen = true;
		}
	}

	if (!seen) {
		runned_instances.push_back(problem);
		num_of_completed_runs.push_back(1);
	}
}

int EvalLog::GetNumCompletedRuns() const {
	int num = 0;
	for (int i = 0; i < num_of_completed_runs.size(); i++)
		num += num_of_completed_runs[i];
	return num;
}

int EvalLog::GetNumRemainingRuns() const {
	return 80 * 30 - GetNumCompletedRuns();
}

int EvalLog::GetNumCompletedRuns(string instance) const {
	for (int i = 0; i < runned_instances.size(); i++) {
		if (runned_instances[i] == instance)
			return num_of_completed_runs[i];
	}
	return 0;
}

int EvalLog::GetNumRemainingRuns(string instance) const {
	return 30 - GetNumCompletedRuns(instance);
}

double EvalLog::GetUsedTimeInSeconds() const {
	time_t curr;
	time(&curr);
	return (double) (curr - start_time);
}

double EvalLog::GetRemainingTimeInSeconds() const {
	return 24 * 3600 - GetUsedTimeInSeconds();
}

// Pre-condition: curr_inst_start_time is initialized
void EvalLog::SetInitialBudget(string instance) {
	curr_inst_budget = 0;
	if (GetNumRemainingRuns() != 0 && GetNumRemainingRuns(instance) != 0) {
		cout << "Num of remaining runs: curr / total = "
				<< GetNumRemainingRuns(instance) << " / "
				<< GetNumRemainingRuns() << endl;
		curr_inst_budget = (24 * 3600 - (curr_inst_start_time - start_time))
				/ GetNumRemainingRuns() * GetNumRemainingRuns(instance);
		if (curr_inst_budget < 0)
			curr_inst_budget = 0;
		if (curr_inst_budget > 18 * 60)
			curr_inst_budget = 18 * 60;
	}
}

double EvalLog::GetRemainingBudget(string instance) const {
	return curr_inst_budget
			- (get_time_second() - EvalLog::curr_inst_start_time);
}

/* =============================================================================
 * Logger class
 * =============================================================================*/

Logger::Logger(DSPOMDP* model, Belief* belief, Solver* solver,
		World* world, string world_type, clock_t start_clockt, ostream* out,
		double target_finish_time, int num_steps) :
		model_(model), world_(world), belief_(belief), start_clockt_(
				start_clockt), world_type_(world_type), step_(0), out_(out), reward_(
				0), total_discounted_reward_(0), total_undiscounted_reward_(0) {
	state_ = world->GetCurrentState();
	target_finish_time_ = target_finish_time;
	if (target_finish_time_ != -1) {
		EvalLog::allocated_time = (target_finish_time_ - get_time_second())
				/ num_steps;
		Globals::config.time_per_move = EvalLog::allocated_time;
		EvalLog::curr_inst_remaining_steps = num_steps;
	}
}

Logger::~Logger() {
}

double Logger::AverageUndiscountedRoundReward() const {
	double sum = 0;
	for (int i = 0; i < undiscounted_round_rewards_.size(); i++) {
		double reward = undiscounted_round_rewards_[i];
		sum += reward;
	}
	return undiscounted_round_rewards_.size() > 0 ?
			(sum / undiscounted_round_rewards_.size()) : 0.0;
}

double Logger::StderrUndiscountedRoundReward() const {
	double sum = 0, sum2 = 0;
	for (int i = 0; i < undiscounted_round_rewards_.size(); i++) {
		double reward = undiscounted_round_rewards_[i];
		sum += reward;
		sum2 += reward * reward;
	}
	int n = undiscounted_round_rewards_.size();
	return n > 0 ? sqrt(sum2 / n / n - sum * sum / n / n / n) : 0.0;
}

double Logger::AverageDiscountedRoundReward() const {
	double sum = 0;
	for (int i = 0; i < discounted_round_rewards_.size(); i++) {
		double reward = discounted_round_rewards_[i];
		sum += reward;
	}
	return discounted_round_rewards_.size() > 0 ?
			(sum / discounted_round_rewards_.size()) : 0.0;
}

double Logger::StderrDiscountedRoundReward() const {
	double sum = 0, sum2 = 0;
	for (int i = 0; i < discounted_round_rewards_.size(); i++) {
		double reward = discounted_round_rewards_[i];
		sum += reward;
		sum2 += reward * reward;
	}
	int n = discounted_round_rewards_.size();
	return n > 0 ? sqrt(sum2 / n / n - sum * sum / n / n / n) : 0.0;
}

void Logger::CheckTargetTime() const {
	if (target_finish_time_ != -1 && get_time_second() > target_finish_time_) {
		if (!Globals::config.silence && out_)
			*out_ << "Exit. (Total time "
					<< (get_time_second() - EvalLog::curr_inst_start_time)
					<< "s exceeded time limit of "
					<< (target_finish_time_ - EvalLog::curr_inst_start_time)
					<< "s)" << endl << "Total time: Real / CPU = "
					<< (get_time_second() - EvalLog::curr_inst_start_time)
					<< " / "
					<< (double(clock() - start_clockt_) / CLOCKS_PER_SEC) << "s"
					<< endl;
		exit(1);
	}
}

void Logger::InitRound(State* state) {
	step_ = 0;

	if (state) {
		state_ = state;
		logi << "[POMDPLogger::InitRound] Created start state." << endl;
		// Print initial state
		if (!Globals::config.silence && out_) {

			if (state_){
				*out_ << "Initial state: " << endl;
				world_->PrintState(*state_, *out_);
				*out_ << endl;
			}
		}
	}

	total_discounted_reward_ = 0;
	total_undiscounted_reward_ = 0;
}

double Logger::EndRound() {
	if (!Globals::config.silence && out_) {
		*out_ << "Total discounted reward = " << total_discounted_reward_
				<< endl << "Total undiscounted reward = "
				<< total_undiscounted_reward_ << endl;
	}

	discounted_round_rewards_.push_back(total_discounted_reward_);
	undiscounted_round_rewards_.push_back(total_undiscounted_reward_);

	return total_undiscounted_reward_;
}

bool Logger::SummarizeStep(int step, int round, bool terminal, ACT_TYPE action,
		OBS_TYPE obs, double step_start_t) {

	//Output step results
	*out_ << "-----------------------------------Round " << round << " Step "
			<< step << "-----------------------------------" << endl;
	if (!Globals::config.silence && out_) {
		*out_ << "- Action = ";
		model_->PrintAction(action, *out_);
	}

	state_ = world_->GetCurrentState();

	if (state_ != NULL) {
		if (!Globals::config.silence && out_) {
			*out_ << "- State:\n";
			model_->PrintState(*state_, *out_);
		}
	}

	if (!Globals::config.silence && out_ && state_ != NULL) {
		*out_ << "- Observation = ";
		model_->PrintObs(*state_, obs, *out_);
	}

	if (state_ != NULL) {
		if (!Globals::config.silence && out_)
			*out_ << "- ObsProb = " << model_->ObsProb(obs, *state_, action)
					<< endl;
	}

	//Record step reward
	if (world_type_ == "pomdp")
		reward_ = static_cast<POMDPWorld*>(world_)->step_reward_;
	else if (state_ != NULL) {
		reward_ = model_->Reward(*state_, action);
		if (reward_ > model_->GetMaxReward()) { //invalid reward from model_->Reward
			reward_ = 0;
			logd
					<< "[Logger::SummarizeStep] Reward function has not been defined in DSPOMDP model"
					<< endl;
		}
	}
	total_discounted_reward_ += Globals::Discount(step_) * reward_;
	total_undiscounted_reward_ += reward_;

	//Report step time
	double step_end_t = get_time_second();
	double step_time = (step_end_t - step_start_t);
	if (terminal) {
		logi << "[RunStep] Time for step: actual / allocated = " << step_time
				<< " / " << EvalLog::allocated_time << endl;
		if (!Globals::config.silence && out_)
			*out_ << endl;
		step_++;
		return true;
	}
	*out_ << endl;
	step_++;

	//Record time per move

	logi << "[main] Time for step: actual / allocated = " << step_time << " / "
			<< EvalLog::allocated_time << endl;

	if (!Globals::config.silence && out_)
		*out_ << "- Reward = " << reward_ << endl << "- Current rewards:"
				<< endl << "  discounted / undiscounted = "
				<< total_discounted_reward_ << " / "
				<< total_undiscounted_reward_ << endl;

	if (target_finish_time_ != -1) {
		if (step_time < 0.99 * EvalLog::allocated_time) {
			if (EvalLog::plan_time_ratio < 1.0)
				EvalLog::plan_time_ratio += 0.01;
			if (EvalLog::plan_time_ratio > 1.0)
				EvalLog::plan_time_ratio = 1.0;
		} else if (step_time > EvalLog::allocated_time) {
			double delta = (step_time - EvalLog::allocated_time)
					/ (EvalLog::allocated_time + 1E-6);
			if (delta < 0.02)
				delta = 0.02; // Minimum reduction per step
			if (delta > 0.05)
				delta = 0.05; // Maximum reduction per step
			EvalLog::plan_time_ratio -= delta;
			// if (EvalLog::plan_time_ratio < 0)
			// EvalLog::plan_time_ratio = 0;
		}

		EvalLog::curr_inst_remaining_budget = target_finish_time_
				- get_time_second();
		EvalLog::curr_inst_remaining_steps--;

		if (EvalLog::curr_inst_remaining_steps <= 0) {
			EvalLog::allocated_time = 0;
		} else {
			EvalLog::allocated_time =
					(EvalLog::curr_inst_remaining_budget - 2.0)
							/ EvalLog::curr_inst_remaining_steps;

			if (EvalLog::allocated_time > 5.0)
				EvalLog::allocated_time = 5.0;
		}

		Globals::config.time_per_move = EvalLog::plan_time_ratio
				* EvalLog::allocated_time;
	}
	logi << "[main] Time per move set to " << Globals::config.time_per_move
			<< endl;
	logi << "[main] Plan time ratio set to " << EvalLog::plan_time_ratio
			<< endl;

	return false;
}

void Logger::PrintStatistics(int num_runs) {
	cout << "\nCompleted " << num_runs << " run(s)." << endl;
	cout << "Average total discounted reward (stderr) = "
			<< AverageDiscountedRoundReward() << " ("
			<< StderrDiscountedRoundReward() << ")" << endl;
	cout << "Average total undiscounted reward (stderr) = "
			<< AverageUndiscountedRoundReward() << " ("
			<< StderrUndiscountedRoundReward() << ")" << endl;
}

} // namespace despot
