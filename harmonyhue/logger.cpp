#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/SyslogAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>

#include <syslog.h>

#include "logger.h"

log4cpp::Category& Logger::Error::root = log4cpp::Category::getRoot();
log4cpp::Category& Logger::Warn::root = log4cpp::Category::getRoot();
log4cpp::Category& Logger::Info::root = log4cpp::Category::getRoot();

Logger::Warn Logger::warn;
Logger::Error Logger::error;
Logger::Info Logger::info;

Logger Logger::logger = Logger("logihue", LOG_DAEMON);

Logger::Logger(const char *log_name, int facility) : root(log4cpp::Category::getRoot()) {
	log4cpp::Appender* console = new log4cpp::OstreamAppender("console", &std::cout);
	console->setLayout(new log4cpp::BasicLayout());
	root.addAppender(console);
	log4cpp::Appender* syslog = new log4cpp::SyslogAppender("syslog", log_name, facility);
	syslog->setLayout(new log4cpp::BasicLayout());
	root.addAppender(syslog);
	root.setPriority(log4cpp::Priority::WARN);
}

void Logger::switchToSyslogOnly() {
	log4cpp::Appender* console = root.getAppender("console");
	root.removeAppender(console);
}

void Logger::daemonized(void) {
	logger.switchToSyslogOnly();
}

void Logger::setToError() {
	logger.root.setPriority(log4cpp::Priority::ERROR);
}

void Logger::setToWarn() {
	logger.root.setPriority(log4cpp::Priority::WARN);
}

void Logger::setToInfo() {
	logger.root.setPriority(log4cpp::Priority::INFO);
}

Logger& Logger::Error::operator<<(std::string message) {
	log4cpp::CategoryStream& s = root << log4cpp::Priority::ERROR << message;
	return Logger::logger;
}

Logger& Logger::Warn::operator<<(std::string message) {
	root << log4cpp::Priority::WARN << message;
	return Logger::logger;
}

Logger& Logger::Info::operator<<(std::string message) {
	root << log4cpp::Priority::INFO << message;
	return Logger::logger;
}

void test() {
    log4cpp::Appender *appender1 = new log4cpp::OstreamAppender("console", &std::cout);
    appender1->setLayout(new log4cpp::BasicLayout());

    log4cpp::Appender *appender2 = new log4cpp::FileAppender("default", "program.log");
    appender2->setLayout(new log4cpp::BasicLayout());
	
	log4cpp::Appender* appender3 = new log4cpp::SyslogAppender("syslog", "harmonyhue", LOG_DAEMON);

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::WARN);
    root.addAppender(appender1);
	root.addAppender(appender3);

    log4cpp::Category& sub1 = log4cpp::Category::getInstance(std::string("sub1"));
    sub1.addAppender(appender2);

// use of functions for logging messages
    root.error("root error");
    root.info("root info");
    sub1.error("sub1 error");
    sub1.warn("sub1 warn");

// printf-style for logging variables
    root.warn("%d + %d == %s ?", 1, 1, "two");

// use of streams for logging messages
    root << log4cpp::Priority::ERROR << "Streamed root error" << "hhh";
    root << log4cpp::Priority::INFO << "Streamed root info";
    sub1 << log4cpp::Priority::ERROR << "Streamed sub1 error";
    sub1 << log4cpp::Priority::WARN << "Streamed sub1 warn";

// or this way:
    root.errorStream() << "Another streamed error";
}
