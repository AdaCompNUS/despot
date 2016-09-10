#include <despot/core/belief.h>
#include <despot/core/pomdp.h>

using namespace std;

namespace despot {

/* =============================================================================
 * ParticleBelief class
 * =============================================================================*/

Belief::Belief(const DSPOMDP* model) :
	model_(model) {
}

Belief::~Belief() {
}

string Belief::text() const {
	return "AbstractBelief";
}

ostream& operator<<(ostream& os, const Belief& belief) {
	os << (&belief)->text();
	return os;
}

vector<State*> Belief::Sample(int num, vector<State*> particles,
	const DSPOMDP* model) {
	double unit = 1.0 / num;
	double mass = Random::RANDOM.NextDouble(0, unit);
	int pos = 0;
	double cur = particles[0]->weight;

	vector<State*> sample;
	for (int i = 0; i < num; i++) {
		while (mass > cur) {
			pos++;
			if (pos == particles.size())
				pos = 0;

			cur += particles[pos]->weight;
		}

		mass += unit;

		State* particle = model->Copy(particles[pos]);
		particle->weight = unit;
		sample.push_back(particle);
	}

	random_shuffle(sample.begin(), sample.end());

	logd << "[Belief::Sample] Sampled " << sample.size() << " particles"
		<< endl;
	for (int i = 0; i < sample.size(); i++) {
		logv << " " << i << " = " << *sample[i] << endl;
	}

	return sample;
}

vector<State*> Belief::Resample(int num, const vector<State*>& belief,
	const DSPOMDP* model, History history, int hstart) {
	double unit = 1.0 / num;
	double mass = Random::RANDOM.NextDouble(0, unit);
	int pos = 0;
	double cur = belief[0]->weight;

	double reward;
	OBS_TYPE obs;

	vector<State*> sample;
	int count = 0;
	double max_wgt = Globals::NEG_INFTY;
	int trial = 0;
	while (count < num && trial < 200 * num) {
		// Pick next particle
		while (mass > cur) {
			pos++;
			if (pos == belief.size())
				pos = 0;

			cur += belief[pos]->weight;
		}
		trial++;

		mass += unit;

		State* particle = model->Copy(belief[pos]);

		// Step through history
		double log_wgt = 0;
		for (int i = hstart; i < history.Size(); i++) {
			model->Step(*particle, Random::RANDOM.NextDouble(),
				history.Action(i), reward, obs);

			double prob = model->ObsProb(history.Observation(i), *particle,
				history.Action(i));
			if (prob > 0) {
				log_wgt += log(prob);
			} else {
				model->Free(particle);
				break;
			}
		}

		// Add to sample if survived
		if (particle->IsAllocated()) {
			count++;

			particle->weight = log_wgt;
			sample.push_back(particle);

			max_wgt = max(log_wgt, max_wgt);
		}

		// Remove particles with very small weights
		if (count == num) {
			for (int i = sample.size() - 1; i >= 0; i--)
				if (sample[i]->weight - max_wgt < log(1.0 / num)) {
					model->Free(sample[i]);
					sample.erase(sample.begin() + i);
					count--;
				}
		}
	}

	double total_weight = 0;
	for (int i = 0; i < sample.size(); i++) {
		sample[i]->weight = exp(sample[i]->weight - max_wgt);
		total_weight += sample[i]->weight;
	}
	for (int i = 0; i < sample.size(); i++) {
		sample[i]->weight = sample[i]->weight / total_weight;
	}

	logd << "[Belief::Resample] Resampled " << sample.size() << " particles"
		<< endl;
	for (int i = 0; i < sample.size(); i++) {
		logv << " " << i << " = " << *sample[i] << endl;
	}

	return sample;
}

vector<State*> Belief::Resample(int num, const DSPOMDP* model,
	const StateIndexer* indexer, int action, OBS_TYPE obs) {
	if (indexer == NULL) {
		loge << "[Belief::Resample] indexer cannot be null" << endl;
		exit(1);
	}

	vector<State*> sample;

	for (int s = 0; s < indexer->NumStates(); s++) {
		const State* state = indexer->GetState(s);
		double prob = model->ObsProb(obs, *state, action);
		if (prob > 0) {
			State* particle = model->Copy(state);
			particle->weight = prob;
			sample.push_back(particle);
		}
	}

	return sample;
}

vector<State*> Belief::Resample(int num, const Belief& belief, History history,
	int hstart) {
	double reward;
	OBS_TYPE obs;

	vector<State*> sample;
	int count = 0;
	int pos = 0;
	double max_wgt = Globals::NEG_INFTY;
	vector<State*> particles;
	int trial = 0;
	while (count < num || trial < 200 * num) {
		// Pick next particle
		if (pos == particles.size()) {
			particles = belief.Sample(num);
			pos = 0;
		}
		State* particle = particles[pos];

		trial++;

		// Step through history
		double log_wgt = 0;
		for (int i = hstart; i < history.Size(); i++) {
			belief.model_->Step(*particle, Random::RANDOM.NextDouble(),
				history.Action(i), reward, obs);

			double prob = belief.model_->ObsProb(history.Observation(i),
				*particle, history.Action(i));
			if (prob > 0) {
				log_wgt += log(prob);
			} else {
				belief.model_->Free(particle);
				break;
			}
		}

		// Add to sample if survived
		if (particle->IsAllocated()) {
			particle->weight = log_wgt;
			sample.push_back(particle);

			max_wgt = max(log_wgt, max_wgt);
			count++;
		}

		// Remove particles with very small weights
		if (count == num) {
			for (int i = sample.size() - 1; i >= 0; i--) {
				if (sample[i]->weight - max_wgt < log(1.0 / num)) {
					belief.model_->Free(sample[i]);
					sample.erase(sample.begin() + i);
					count--;
				}
			}
		}

		pos++;
	}

	// Free unused particles
	for (int i = pos; i < particles.size(); i++)
		belief.model_->Free(particles[i]);

	double total_weight = 0;
	for (int i = 0; i < sample.size(); i++) {
		sample[i]->weight = exp(sample[i]->weight - max_wgt);
		total_weight += sample[i]->weight;
	}
	for (int i = 0; i < sample.size(); i++) {
		sample[i]->weight = sample[i]->weight / total_weight;
	}

	logd << "[Belief::Resample] Resampled " << sample.size() << " particles"
		<< endl;
	for (int i = 0; i < sample.size(); i++) {
		logv << " " << i << " = " << *sample[i] << endl;
	}

	return sample;
}

/* =============================================================================
 * ParticleBelief class
 * =============================================================================*/

ParticleBelief::ParticleBelief(vector<State*> particles, const DSPOMDP* model,
	Belief* prior, bool split) :
	Belief(model),
	particles_(particles),
	num_particles_(particles.size()),
	prior_(prior),
	split_(split),
	state_indexer_(NULL) {

	if (fabs(State::Weight(particles) - 1.0) > 1e-6) {
		loge << "[ParticleBelief::ParticleBelief] Particle weights sum to " << State::Weight(particles) << " instead of 1" << endl;
		exit(1);
	}

	if (split) {
		// Maintain more particles to avoid degeneracy
		while (2 * num_particles_ < 5000)
			num_particles_ *= 2;
		if (particles_.size() < num_particles_) {
			logi << "[ParticleBelief::ParticleBelief] Splitting " << particles_.size()
				<< " particles into " << num_particles_ << " particles." << endl;
			vector<State*> new_particles;
			int n = num_particles_ / particles_.size();
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < particles_.size(); j++) {
					State* particle = particles_[j];
					State* copy = model_->Copy(particle);
					copy->weight /= n;
					new_particles.push_back(copy);
				}
			}

			for (int i = 0; i < particles_.size(); i++)
				model_->Free(particles_[i]);

			particles_ = new_particles;
		}
	}

	if (fabs(State::Weight(particles) - 1.0) > 1e-6) {
		loge << "[ParticleBelief::ParticleBelief] Particle weights sum to " << State::Weight(particles) << " instead of 1" << endl;
		exit(1);
	}

	random_shuffle(particles_.begin(), particles_.end());
	// cerr << "Number of particles in initial belief: " << particles_.size() << endl;

	if (prior_ == NULL) {
		for (int i = 0; i < particles.size(); i++)
			initial_particles_.push_back(model_->Copy(particles[i]));
	}
}

ParticleBelief::~ParticleBelief() {
	for (int i = 0; i < particles_.size(); i++) {
		model_->Free(particles_[i]);
	}

	for (int i = 0; i < initial_particles_.size(); i++) {
		model_->Free(initial_particles_[i]);
	}
}

void ParticleBelief::state_indexer(const StateIndexer* indexer) {
	state_indexer_ = indexer;
}

const vector<State*>& ParticleBelief::particles() const {
	return particles_;
}

vector<State*> ParticleBelief::Sample(int num) const {
	return Belief::Sample(num, particles_, model_);
}

void ParticleBelief::Update(int action, OBS_TYPE obs) {
	history_.Add(action, obs);

	vector<State*> updated;
	double total_weight = 0;
	double reward;
	OBS_TYPE o;
	// Update particles
	for (int i = 0; i <particles_.size(); i++) {
		State* particle = particles_[i];
		bool terminal = model_->Step(*particle, Random::RANDOM.NextDouble(),
			action, reward, o);
		double prob = model_->ObsProb(obs, *particle, action);

		if (!terminal && prob) { // Terminal state is not required to be explicitly represented and may not have any observation
			particle->weight *= prob;
			total_weight += particle->weight;
			updated.push_back(particle);
		} else {
			model_->Free(particle);
		}
	}

	logd << "[ParticleBelief::Update] " << updated.size()
		<< " particles survived among " << particles_.size() << endl;
	particles_ = updated;

	// Resample if the particle set is empty
	if (particles_.size() == 0) {
		logw << "Particle set is empty!" << endl;
		if (prior_ != NULL) {
			logw
				<< "Resampling by drawing random particles from prior which are consistent with history"
				<< endl;
			particles_ = Resample(num_particles_, *prior_, history_);
		} else {
			logw
				<< "Resampling by searching initial particles which are consistent with history"
				<< endl;
			particles_ = Resample(num_particles_, initial_particles_, model_,
				history_);
		}

		if (particles_.size() == 0 && state_indexer_ != NULL) {
			logw
				<< "Resampling by searching states consistent with last (action, observation) pair"
				<< endl;
			particles_ = Resample(num_particles_, model_, state_indexer_,
				action, obs);
		}

		if (particles_.size() == 0) {
			logw << "Resampling failed - Using initial particles" << endl;
			for (int i = 0; i < initial_particles_.size(); i ++)
				particles_.push_back(model_->Copy(initial_particles_[i]));
		}
		
		//Update total weight so that effective number of particles are computed correctly 
		total_weight = 0;
                for (int i = 0; i < particles_.size(); i++) {
		    State* particle = particles_[i];
                    total_weight = total_weight + particle->weight;
                }
	}

	
	double weight_square_sum = 0;
	for (int i = 0; i < particles_.size(); i++) {
		State* particle = particles_[i];
		particle->weight /= total_weight;
		weight_square_sum += particle->weight * particle->weight;
	}

	// Resample if the effective number of particles is "small"
	double num_effective_particles = 1.0 / weight_square_sum;
	if (num_effective_particles < num_particles_ / 2.0) {
		vector<State*> new_belief = Belief::Sample(num_particles_, particles_,
			model_);
		for (int i = 0; i < particles_.size(); i++)
			model_->Free(particles_[i]);

		particles_ = new_belief;
	}
}

Belief* ParticleBelief::MakeCopy() const {
	vector<State*> copy;
	for (int i = 0; i < particles_.size(); i++) {
		copy.push_back(model_->Copy(particles_[i]));
	}

	return new ParticleBelief(copy, model_, prior_, split_);
}

string ParticleBelief::text() const {
	ostringstream oss;
	map<string, double> pdf;
	for (int i = 0; i < particles_.size(); i++) {
		pdf[particles_[i]->text()] += particles_[i]->weight;
	}

	oss << "pdf for " << particles_.size() << " particles:" << endl;
	vector<pair<string, double> > pairs = SortByValue(pdf);
	for (int i = 0; i < pairs.size(); i++) {
		pair<string, double> pair = pairs[i];
		oss << " " << pair.first << " = " << pair.second << endl;
	}
	return oss.str();
}

} // namespace despot
