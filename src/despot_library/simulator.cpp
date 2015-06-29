#include "simulator.h"

/* =============================================================================
 * IPPCLog class
 * =============================================================================*/

time_t IPPCLog::start_time = 0;
double IPPCLog::curr_inst_start_time = 0;
double IPPCLog::curr_inst_target_time = 0;
double IPPCLog::curr_inst_budget = 0;
double IPPCLog::curr_inst_remaining_budget = 0;
int IPPCLog::curr_inst_steps = 0;
int IPPCLog::curr_inst_remaining_steps = 0;
double IPPCLog::allocated_time = 1.0;
double IPPCLog::plan_time_ratio = 1.0;

IPPCLog::IPPCLog(string log_file) :
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

void IPPCLog::Save() {
	ofstream fout(log_file_.c_str(), ofstream::out);
	fout << start_time << endl;
	fout << runned_instances.size() << endl;
	for (int i = 0; i < runned_instances.size(); i++)
		fout << runned_instances[i] << " " << num_of_completed_runs[i] << endl;
	fout.close();
}

void IPPCLog::IncNumOfCompletedRuns(string problem) {
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

int IPPCLog::GetNumCompletedRuns() const {
	int num = 0;
	for (int i = 0; i < num_of_completed_runs.size(); i++)
		num += num_of_completed_runs[i];
	return num;
}

int IPPCLog::GetNumRemainingRuns() const {
	return 80 * 30 - GetNumCompletedRuns();
}

int IPPCLog::GetNumCompletedRuns(string instance) const {
	for (int i = 0; i < runned_instances.size(); i++) {
		if (runned_instances[i] == instance)
			return num_of_completed_runs[i];
	}
	return 0;
}

int IPPCLog::GetNumRemainingRuns(string instance) const {
	return 30 - GetNumCompletedRuns(instance);
}

double IPPCLog::GetUsedTimeInSeconds() const {
	time_t curr;
	time(&curr);
	return (double) (curr - start_time);
}

double IPPCLog::GetRemainingTimeInSeconds() const {
	return 24 * 3600 - GetUsedTimeInSeconds();
}

// Pre-condition: curr_inst_start_time is initialized
void IPPCLog::SetInitialBudget(string instance) {
	curr_inst_budget = 0;
	if (GetNumRemainingRuns() != 0 && GetNumRemainingRuns(instance) != 0) {
		cout << "Num of remaining runs: curr / total = "
			<< GetNumRemainingRuns(instance) << " / " << GetNumRemainingRuns()
			<< endl;
		curr_inst_budget = (24 * 3600 - (curr_inst_start_time - start_time))
			/ GetNumRemainingRuns() * GetNumRemainingRuns(instance);
		if (curr_inst_budget < 0)
			curr_inst_budget = 0;
		if (curr_inst_budget > 18 * 60)
			curr_inst_budget = 18 * 60;
	}
}

double IPPCLog::GetRemainingBudget(string instance) const {
	return curr_inst_budget
		- (get_time_second() - IPPCLog::curr_inst_start_time);
}

/* =============================================================================
 * Simulator class
 * =============================================================================*/

Simulator::Simulator(DSPOMDP* model, string belief_type, Solver* solver,
	clock_t start_clockt, ostream* out) :
	model_(model),
	belief_type_(belief_type),
	solver_(solver),
	start_clockt_(start_clockt),
	target_finish_time_(-1),
	out_(out) {
}

Simulator::~Simulator() {
}

bool Simulator::RunStep() {
	if (target_finish_time_ != -1 && get_time_second() > target_finish_time_) {
		if (!Globals::config.silence && out_)
			*out_ << "Exit. (Total time "
				<< (get_time_second() - IPPCLog::curr_inst_start_time)
				<< "s exceeded time limit of "
				<< (target_finish_time_ - IPPCLog::curr_inst_start_time) << "s)"
				<< endl
				<< "Total time: Real / CPU = "
				<< (get_time_second() - IPPCLog::curr_inst_start_time) << " / "
				<< (double(clock() - start_clockt_) / CLOCKS_PER_SEC) << "s"
				<< endl;
		exit(1);
	}

	double step_start_t = get_time_second();

	double start_t = get_time_second();
	int action = solver_->Search().action;
	double end_t = get_time_second();
	logi << "[RunStep] Time spent in " << typeid(*solver_).name()
		<< "::Search(): " << (end_t - start_t) << endl;

	double reward;
	OBS_TYPE obs;
	start_t = get_time_second();
	bool terminal = ExecuteAction(action, reward, obs);
	end_t = get_time_second();
	logi << "[RunStep] Time spent in ExecuteAction(): " << (end_t - start_t)
		<< endl;

	start_t = get_time_second();
	if (!Globals::config.silence && out_) {
		*out_ << "- Action = ";
		model_->PrintAction(action, *out_);
	}

	if (state_ != NULL) {
		if (!Globals::config.silence && out_) {
			*out_ << "- State:\n";
			model_->PrintState(*state_, *out_);
		}
	}

	if (!Globals::config.silence && out_) {
		*out_ << "- Observation = ";
		model_->PrintObs(*state_, obs, *out_);
	}

	if (state_ != NULL) {
		if (!Globals::config.silence && out_)
			*out_ << "- ObsProb = " << model_->ObsProb(obs, *state_, action)
				<< endl;
	}

	ReportStepReward();
	end_t = get_time_second();

	double step_end_t;
	if (terminal) {
		step_end_t = get_time_second();
		logi << "[RunStep] Time for step: actual / allocated = "
			<< (step_end_t - step_start_t) << " / " << IPPCLog::allocated_time
			<< endl;
		if (!Globals::config.silence && out_)
			*out_ << endl;
		step_++;
		return true;
	}

	start_t = get_time_second();
	solver_->Update(action, obs);
	end_t = get_time_second();
	logi << "[RunStep] Time spent in Update(): " << (end_t - start_t) << endl;

	step_++;
	return false;
}

double Simulator::AverageUndiscountedRoundReward() const {
	double sum = 0;
	for (int i = 0; i < undiscounted_round_rewards_.size(); i++) {
		double reward = undiscounted_round_rewards_[i];
		sum += reward;
	}
	return undiscounted_round_rewards_.size() > 0 ? (sum / undiscounted_round_rewards_.size()) : 0.0;
}

double Simulator::StderrUndiscountedRoundReward() const {
	double sum = 0, sum2 = 0;
	for (int i = 0; i < undiscounted_round_rewards_.size(); i++) {
		double reward = undiscounted_round_rewards_[i];
		sum += reward;
		sum2 += reward * reward;
	}
	int n = undiscounted_round_rewards_.size();
	return n > 0 ? sqrt(sum2 / n / n - sum * sum / n / n / n) : 0.0;
}


double Simulator::AverageDiscountedRoundReward() const {
	double sum = 0;
	for (int i = 0; i < discounted_round_rewards_.size(); i++) {
		double reward = discounted_round_rewards_[i];
		sum += reward;
	}
	return discounted_round_rewards_.size() > 0 ? (sum / discounted_round_rewards_.size()) : 0.0;
}

double Simulator::StderrDiscountedRoundReward() const {
	double sum = 0, sum2 = 0;
	for (int i = 0; i < discounted_round_rewards_.size(); i++) {
		double reward = discounted_round_rewards_[i];
		sum += reward;
		sum2 += reward * reward;
	}
	int n = discounted_round_rewards_.size();
	return n > 0 ? sqrt(sum2 / n / n - sum * sum / n / n / n) : 0.0;
}

void Simulator::ReportStepReward() {
	// NOTE: The rewards are incorrectly computed at last step in IPPC
	if (!Globals::config.silence && out_)
		*out_ << "- Reward = " << reward_ << endl
			<< "- Current rewards:" << endl
			<< "  discounted / undiscounted = " << total_discounted_reward_
			<< " / " << total_undiscounted_reward_ << endl;
}

/* =============================================================================
 * IPPCSimulator class
 * =============================================================================*/
/*
IPPCSimulator::IPPCSimulator(DSPOMDP* model, string belief_type, Solver* solver,
	clock_t start_clockt, string hostname, string port, string log,
	ostream* out) :
	Simulator(model, belief_type, solver, start_clockt, out),
	pomdpx_(static_cast<POMDPX*>(model)),
	log_(log),
	hostname_(hostname),
	port_(port) {
}

IPPCSimulator::~IPPCSimulator() {
}

int IPPCSimulator::Handshake(string instance) {
	int num_remaining_runs = log_.GetNumRemainingRuns(instance);
	if (num_remaining_runs == 0) {
		return 0;
	}

	double start_t = get_time_second();
	instance_ = instance;

	client_ = new Client();
	client_->setHostName(hostname_);
	client_->setPort(port_);

	client_->initializeSocket();
	client_->connectToServer();

	client_->sendMessage(client_->createSessionRequestMes(instance));

	string sessionInitMes = client_->recvMessage();

	if (!Globals::config.silence && out_) {
		*out_ << sessionInitMes << endl;
	}

	client_->processSessionInitMes(sessionInitMes);
	double end_t = get_time_second();

	if (!Globals::config.silence && out_) {
		*out_ << "Time for handsake " << (end_t - start_t) << endl;
	}

	log_.SetInitialBudget(instance);
	IPPCLog::curr_inst_steps = num_remaining_runs * Globals::config.sim_len;
	IPPCLog::curr_inst_remaining_steps = num_remaining_runs
		* Globals::config.sim_len;
	IPPCLog::curr_inst_target_time = IPPCLog::curr_inst_budget
		/ IPPCLog::curr_inst_steps;
	UpdateTimeInfo(instance);
	IPPCLog::plan_time_ratio = 1.0;
	Globals::config.time_per_move = IPPCLog::plan_time_ratio
		* IPPCLog::allocated_time;

	return num_remaining_runs;
}

void IPPCSimulator::InitRound() {
	step_ = 0;
	state_ = NULL;

	double start_t, end_t;

	// Initial belief
	start_t = get_time_second();
	delete solver_->belief();
	end_t = get_time_second();
	logi << "[IPPCSimulator::InitRound] Deleted initial belief in "
		<< (end_t - start_t) << "s" << endl;

	start_t = get_time_second();
	Belief* belief = model_->InitialBelief(NULL, belief_type_);
	end_t = get_time_second();
	logi << "[IPPCSimulator::InitRound] Initialized initial belief: "
		<< typeid(*belief).name() << " in " << (end_t - start_t) << "s" << endl;

	solver_->belief(belief);

	// Initiate a round with server
	start_t = get_time_second();
	client_->sendMessage(client_->createRoundRequestMes());
	string roundMes = client_->recvMessageTwice();
	end_t = get_time_second();
	logi << "[IPPCSimulator::InitRound] Time for startround msg "
		<< (end_t - start_t) << "s" << endl;

	total_discounted_reward_ = 0;
	total_undiscounted_reward_ = 0;
}

double IPPCSimulator::EndRound() {
	double start_t = get_time_second();

	string roundEndMes = client_->recvMessage();
	double round_reward = client_->processRoundEndMes(roundEndMes);

	if (!Globals::config.silence && out_) {
		*out_ << "Total undiscounted reward = " << round_reward << endl;
	}

	log_.IncNumOfCompletedRuns(instance_);
	log_.Save();

	double end_t = get_time_second();

	if (!Globals::config.silence && out_) {
		*out_ << "Time for endround msg (save log) " << (end_t - start_t)
			<< endl;
	}

	discounted_round_rewards_.push_back(total_discounted_reward_);
	undiscounted_round_rewards_.push_back(round_reward);

	return round_reward;
}

bool IPPCSimulator::ExecuteAction(int action, double& reward, OBS_TYPE& obs) {
	double start_t = get_time_second();

	client_->sendMessage(
		client_->createActionMes(pomdpx_->GetActionName(),
			pomdpx_->GetEnumedAction(action)));

	if (step_ == Globals::config.sim_len - 1) {
		return true;
	}

	string turnMes = client_->recvMessage();

	//get step reward from turn message: added by wkg
	reward = client_->getStepReward(turnMes);
	reward_ = reward;
	total_discounted_reward_ += Discount(step_) * reward;
	total_undiscounted_reward_ += reward;

	map<string, string> observs = client_->processTurnMes(turnMes);
	obs = pomdpx_->GetPOMDPXObservation(observs);

	double end_t = get_time_second();

	if (!Globals::config.silence && out_) {
		*out_ << "Time for executing action " << (end_t - start_t) << endl;
	}

	return false;
}

double IPPCSimulator::End() {
	double start_t = get_time_second();

	string sessionEndMes = client_->recvMessage();
	double total_reward = client_->processSessionEndMes(sessionEndMes);
	client_->closeConnection();
	delete client_;

	double end_t = get_time_second();

	if (!Globals::config.silence && out_) {
		*out_ << "Time for endsession " << (end_t - start_t) << endl
			<< "Total reward for all runs = " << total_reward << endl
			<< "Total time: Real / CPU = "
			<< (get_time_second() - IPPCLog::curr_inst_start_time) << " / "
			<< (double(clock() - start_clockt_) / CLOCKS_PER_SEC) << "s"
			<< endl;
	}

	return total_reward;
}

void IPPCSimulator::UpdateTimeInfo(string instance) {
	IPPCLog::curr_inst_remaining_budget = log_.GetRemainingBudget(instance);
	if (IPPCLog::curr_inst_remaining_budget <= 0) {
		IPPCLog::curr_inst_remaining_budget = 0;
	}

	if (IPPCLog::curr_inst_remaining_steps <= 0) {
		IPPCLog::allocated_time = 0;
	} else {
		IPPCLog::allocated_time = (IPPCLog::curr_inst_remaining_budget - 2.0)
			/ IPPCLog::curr_inst_remaining_steps;

		if (IPPCLog::allocated_time > 5.0)
			IPPCLog::allocated_time = 5.0;
	}
}

void IPPCSimulator::UpdateTimePerMove(double step_time) {
	if (step_time < 0.99 * IPPCLog::allocated_time) {
		if (IPPCLog::plan_time_ratio < 1.0)
			IPPCLog::plan_time_ratio += 0.01;
		if (IPPCLog::plan_time_ratio > 1.0)
			IPPCLog::plan_time_ratio = 1.0;
	} else if (step_time > IPPCLog::allocated_time) {
		double delta = (step_time - IPPCLog::allocated_time)
			/ (IPPCLog::allocated_time + 1E-6);
		if (delta < 0.02)
			delta = 0.02; // Minimum reduction per step
		if (delta > 0.05)
			delta = 0.05; // Maximum reduction per step
		IPPCLog::plan_time_ratio -= delta;
		// if (IPPCLog::plan_time_ratio < 0)
		// IPPCLog::plan_time_ratio = 0;
	}

	IPPCLog::curr_inst_remaining_budget = log_.GetRemainingBudget(instance_);
	IPPCLog::curr_inst_remaining_steps--;

	UpdateTimeInfo(instance_);
	Globals::config.time_per_move = IPPCLog::plan_time_ratio
		* IPPCLog::allocated_time;

	if (!Globals::config.silence && out_) {
		*out_
			<< "Total time: curr_inst / inst_target / remaining / since_start = "
			<< (get_time_second() - IPPCLog::curr_inst_start_time) << " / "
			<< (IPPCLog::curr_inst_target_time
				* (IPPCLog::curr_inst_steps - IPPCLog::curr_inst_remaining_steps))
			<< " / " << IPPCLog::curr_inst_remaining_budget << " / "
			<< (get_time_second() - IPPCLog::start_time) << endl;
	}
}
*/
/* =============================================================================
 * POMDPSimulator class
 * =============================================================================*/

POMDPSimulator::POMDPSimulator(DSPOMDP* model, string belief_type,
	Solver* solver, clock_t start_clockt, ostream* out,
	double target_finish_time, int num_steps) :
	Simulator(model, belief_type, solver, start_clockt, out),
	random_((unsigned) 0) {
	target_finish_time_ = target_finish_time;

	if (target_finish_time_ != -1) {
		IPPCLog::allocated_time = (target_finish_time_ - get_time_second())
			/ num_steps;
		Globals::config.time_per_move = IPPCLog::allocated_time;
		IPPCLog::curr_inst_remaining_steps = num_steps;
	}
}

POMDPSimulator::~POMDPSimulator() {
}

int POMDPSimulator::Handshake(string instance) {
	return -1; // Not to be used
}

void POMDPSimulator::InitRound() {
	step_ = 0;

	double start_t, end_t;
	// Initial state
	state_ = model_->CreateStartState();
	logi << "[POMDPSimulator::InitRound] Created start state." << endl;
	if (!Globals::config.silence && out_) {
		*out_ << "Initial state: " << endl;
		model_->PrintState(*state_, *out_);
		*out_ << endl;
	}

	// Initial belief
	start_t = get_time_second();
	delete solver_->belief();
	end_t = get_time_second();
	logi << "[POMDPSimulator::InitRound] Deleted old belief in "
		<< (end_t - start_t) << "s" << endl;

	start_t = get_time_second();
	Belief* belief = model_->InitialBelief(state_, belief_type_);
	end_t = get_time_second();
	logi << "[POMDPSimulator::InitRound] Created intial belief "
		<< typeid(*belief).name() << " in " << (end_t - start_t) << "s" << endl;

	solver_->belief(belief);

	total_discounted_reward_ = 0;
	total_undiscounted_reward_ = 0;
}

double POMDPSimulator::EndRound() {
	if (!Globals::config.silence && out_) {
		*out_ << "Total discounted reward = " << total_discounted_reward_ << endl
			<< "Total undiscounted reward = " << total_undiscounted_reward_ << endl;
	}

	discounted_round_rewards_.push_back(total_discounted_reward_);
	undiscounted_round_rewards_.push_back(total_undiscounted_reward_);

	return total_undiscounted_reward_;
}

bool POMDPSimulator::ExecuteAction(int action, double& reward, OBS_TYPE& obs) {
	double random_num = random_.NextDouble();
	bool terminal = model_->Step(*state_, random_num, action, reward, obs);

	reward_ = reward;
	total_discounted_reward_ += Discount(step_) * reward;
	total_undiscounted_reward_ += reward;

	return terminal;
}

double POMDPSimulator::End() {
	return 0; // Not to be used
}

void POMDPSimulator::UpdateTimePerMove(double step_time) {
	if (target_finish_time_ != -1) {
		if (step_time < 0.99 * IPPCLog::allocated_time) {
			if (IPPCLog::plan_time_ratio < 1.0)
				IPPCLog::plan_time_ratio += 0.01;
			if (IPPCLog::plan_time_ratio > 1.0)
				IPPCLog::plan_time_ratio = 1.0;
		} else if (step_time > IPPCLog::allocated_time) {
			double delta = (step_time - IPPCLog::allocated_time)
				/ (IPPCLog::allocated_time + 1E-6);
			if (delta < 0.02)
				delta = 0.02; // Minimum reduction per step
			if (delta > 0.05)
				delta = 0.05; // Maximum reduction per step
			IPPCLog::plan_time_ratio -= delta;
			// if (IPPCLog::plan_time_ratio < 0)
			// IPPCLog::plan_time_ratio = 0;
		}

		IPPCLog::curr_inst_remaining_budget = target_finish_time_
			- get_time_second();
		IPPCLog::curr_inst_remaining_steps--;

		if (IPPCLog::curr_inst_remaining_steps <= 0) {
			IPPCLog::allocated_time = 0;
		} else {
			IPPCLog::allocated_time =
				(IPPCLog::curr_inst_remaining_budget - 2.0)
					/ IPPCLog::curr_inst_remaining_steps;

			if (IPPCLog::allocated_time > 5.0)
				IPPCLog::allocated_time = 5.0;
		}

		Globals::config.time_per_move = IPPCLog::plan_time_ratio
			* IPPCLog::allocated_time;
	}
}
