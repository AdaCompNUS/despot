#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

class log_ostream: public std::ostream {
private:
	class log_buf: public std::stringbuf {
	private:
		ostream& out_;
		string marker_;

	public:
		log_buf(ostream& out, string marker = "");
		~log_buf();

		virtual streambuf* setbuf(char* s, streamsize n);
		virtual int sync();
	};

	log_buf buffer_;

protected:

public:
	log_ostream(ostream& out, string marker = "");
};

class logging {
private:
	static int verbosity_;

	static vector<log_ostream*> streams_;
	static vector<log_ostream*> InitializeLogStreams();
	static const string markers_[];

public:
	static const int NONE, ERROR, WARN, INFO, DEBUG, VERBOSE;

public:
	static void level(int verbosity);
	static int level();
	static log_ostream& stream(int level);
	static void stream(int level, ostream& out);
};

#define LOG(lv) \
if (logging::level() < logging::ERROR || logging::level() < lv) ; \
else logging::stream(lv)

#define loge LOG(logging::ERROR)
#define logw LOG(logging::WARN)
#define logi LOG(logging::INFO)
#define logd LOG(logging::DEBUG)
#define logv LOG(logging::VERBOSE)

#define default_out \
if (Globals::config.silence) ; \
else cout

#endif
