#include <despot/random_streams.h>
#include <despot/util/seeds.h>
#include <vector>

using namespace std;

namespace despot {

RandomStreams::RandomStreams(int num_streams, int length) :
	position_(0) {
	vector<unsigned> seeds = Seeds::Next(num_streams);

	streams_.resize(num_streams);
	for (int i = 0; i < num_streams; i++) {
		Random random(seeds[i]);
		streams_[i].resize(length);
		for (int j = 0; j < length; j++)
			streams_[i][j] = random.NextDouble();
	}
}

int RandomStreams::NumStreams() const {
	return streams_.size();
}

int RandomStreams::Length() const {
	return streams_.size() > 0 ? streams_[0].size() : 0;
}

void RandomStreams::Advance() const {
	position_++;
}
void RandomStreams::Back() const {
	position_--;
}

void RandomStreams::position(int value) const {
	position_ = value;
}
int RandomStreams::position() const {
	return position_;
}

bool RandomStreams::Exhausted() const {
	return position_ > Length() - 1;
}

double RandomStreams::Entry(int stream) const {
	return streams_[stream][position_];
}

double RandomStreams::Entry(int stream, int position) const {
	return streams_[stream][position];
}

ostream& operator<<(ostream& os, const RandomStreams& stream) {
	for (int i = 0; i < stream.NumStreams(); i++) {
		os << "Stream " << i << ":";
		for (int j = 0; j < stream.Length(); j++) {
			os << " " << stream.Entry(i, j);
		}
		os << endl;
	}
	return os;
}

} // namespace despot
