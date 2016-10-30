#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/SyslogAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/Priority.hh>

#include <syslog.h>

#include "logger.h"

Logger Logger::logger = Logger("logihue", LOG_DAEMON);

Logger::Logger(const char *log_name, int facility) : root(log4cpp::Category::getRoot()) {
	log4cpp::Appender* console = new log4cpp::OstreamAppender("console", &std::cout);
	console->setLayout(new log4cpp::SimpleLayout());
	root.addAppender(console);
	log4cpp::Appender* syslog = new log4cpp::SyslogAppender("syslog", log_name, facility);
	syslog->setLayout(new log4cpp::SimpleLayout());
	root.addAppender(syslog);
	root.setPriority(log4cpp::Priority::WARN);
}


void Logger::switchToSyslogOnly() {
	log4cpp::Appender* console = root.getAppender("console");
	root.removeAppender(console);
}

void Logger::daemonized() {
	logger.switchToSyslogOnly();
}
