#ifndef LOGGER_H
#define LOGGER_H

#include <string>


namespace log4cpp {
	class Category;
	class CategoryStream;
}

class Logger {
public:
	class Error {
	private:
		static log4cpp::Category& root;
	public:
		Logger& operator<<(std::string message);
	};
	
	class Warn {
	private:
		static log4cpp::Category& root;
	public:
		Logger& operator<<(std::string message);
	};
	
	class Info {
	private:
		static log4cpp::Category& root;
	public:
		Logger& operator<<(std::string message);
	};

private:
	Logger(const char *log_name, int facility);
	void switchToSyslogOnly();
	
public:
	static Logger* instance();

	static void setToError();
	static void setToWarn();
	static void setToInfo();
	
	static void daemonized();
	
	static Error error;
	static Warn warn;
	static Info info;

private:
	static Logger logger;
	log4cpp::Category& root;
};


void test();

#endif
