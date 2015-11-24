#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <sstream>
#include <vector>

namespace despot {

class log_ostream: public std::ostream {
private:
	class log_buf: public std::stringbuf {
	private:
    std::ostream& out_;
		std::string marker_;

	public:
		log_buf(std::ostream& out, std::string marker = "");
		~log_buf();

		virtual std::streambuf* setbuf(char* s, std::streamsize n);
		virtual int sync();
	};

	log_buf buffer_;

protected:

public:
	log_ostream(std::ostream& out, std::string marker = "");
};

class logging {
private:
	static int verbosity_;

	static std::vector<log_ostream*> streams_;
	static std::vector<log_ostream*> InitializeLogStreams();
	static const std::string markers_[];

public:
	static const int NONE, ERROR, WARN, INFO, DEBUG, VERBOSE;

public:
	static void level(int verbosity);
	static int level();
	static log_ostream& stream(int level);
	static void stream(int level, std::ostream& out);
};

} // namespace despot

#define LOG(lv) \
if (despot::logging::level() < despot::logging::ERROR || despot::logging::level() < lv) ; \
else despot::logging::stream(lv)

#define loge LOG(despot::logging::ERROR)
#define logw LOG(despot::logging::WARN)
#define logi LOG(despot::logging::INFO)
#define logd LOG(despot::logging::DEBUG)
#define logv LOG(despot::logging::VERBOSE)

#define default_out \
if (Globals::config.silence) ; \
else cout

#endif
