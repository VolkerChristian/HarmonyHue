#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>

#define LogFatal Logger::logger.root.fatalStream()
#define LogCrit Logger::logger.root.critStream()
#define LogError Logger::logger.root.errorStream()
#define LogWarn Logger::logger.root.warnStream()
#define LogNotice Logger::logger.root.noticeStream()
#define LogInfo Logger::logger.root.infoStream()
#define LogDebug Logger::logger.root.debugStream()


class Logger {
public:
    Logger(const char *log_name, int facility);
    
    log4cpp::Category& root;
    
    
    static void setToFatal() {
        logger.root.setPriority(log4cpp::Priority::FATAL);
    }
    
    static void setToCrit() {
        logger.root.setPriority(log4cpp::Priority::CRIT);
    }
	
	static void setToError() {
        logger.root.setPriority(log4cpp::Priority::ERROR);
    }
	
	static void setToWarn() {
        logger.root.setPriority(log4cpp::Priority::WARN);
    }
    
    static void setToNotice() {
        logger.root.setPriority(log4cpp::Priority::NOTICE);
    }
	
	static void setToInfo() {
        logger.root.setPriority(log4cpp::Priority::INFO);
    }
    
    static void setToDebug() {
        logger.root.setPriority(log4cpp::Priority::DEBUG);
    }

    void switchToSyslogOnly();
    static void daemonized();
    
    static Logger logger;
};

#endif
