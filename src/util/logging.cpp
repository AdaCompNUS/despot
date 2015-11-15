#include <despot/util/logging.h>
// #include <thread>

using namespace std;

namespace despot {

const string logging::markers_[] = { "NONE", "ERROR", "WARN", "INFO", "DEBUG",
	"VERBOSE" };
const int logging::NONE = 0;
const int logging::ERROR = 1;
const int logging::WARN = 2;
const int logging::INFO = 3;
const int logging::DEBUG = 4;
const int logging::VERBOSE = 5;

log_ostream::log_ostream(ostream& out, string marker) :
	ostream(&buffer_),
	buffer_(out, marker) {
}

log_ostream::log_buf::log_buf(ostream& out, string marker) :
	out_(out),
	marker_(marker) {
}

log_ostream::log_buf::~log_buf() {
	// pubsync();
}

streambuf* log_ostream::log_buf::setbuf(char* s, streamsize n) {
	return this;
}

int log_ostream::log_buf::sync() {
	//NOTE: disabled c++11 feature
	// out_ << marker_ << "-" << this_thread::get_id() << ": " << str();
	out_ << marker_ << ": " << str();
	str("");
	return !out_;
}

int logging::verbosity_ = ERROR;

void logging::level(int verbosity) {
	verbosity_ = verbosity;
}

int logging::level() {
	return verbosity_;
}

log_ostream& logging::stream(int level) {
	return *(streams_[level]);
}

void logging::stream(int level, ostream& out) {
	if (level >= ERROR && level <= VERBOSE) {
		streams_[level] = new log_ostream(out, markers_[level]);
	}
}

vector<log_ostream*> logging::InitializeLogStreams() {
	vector<log_ostream*> streams(6);

	streams[NONE] = NULL;
	streams[ERROR] = new log_ostream(cerr, markers_[ERROR]);
	streams[WARN] = new log_ostream(cerr, markers_[WARN]);
	streams[INFO] = new log_ostream(cerr, markers_[INFO]);
	streams[DEBUG] = new log_ostream(cerr, markers_[DEBUG]);
	streams[VERBOSE] = new log_ostream(cerr, markers_[VERBOSE]);

	return streams;
}

vector<log_ostream*> logging::streams_ = logging::InitializeLogStreams();

} // namespace despot
