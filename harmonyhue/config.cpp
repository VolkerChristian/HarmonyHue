#include "config.h"

#include <algorithm>
#include <fstream>

#include "logger.h"
	
Config* Config::config = 0;

int Config::oLoad() {
	int ret = 0;
	
	std::ifstream configStream(configFile);
	
	char cEntry[256];
	char cValue[256];
	
	if (configStream.is_open()) {
		while(!configStream.eof()) {
			configStream.getline(cEntry, 256, ':');
			configStream.getline(cValue, 256);
			std::string entry = cEntry;
			std::string value = cValue;
			if (!entry.empty()) {
				entry.erase(std::remove_if(entry.begin(), entry.end(), ::isspace), entry.end());
				value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

				configMap[entry] = value;
			}
		}
		configStream.close();
	} else {
		ret = 1;
	}
	
	return ret;
}

int Config::oSave() {
	int ret = 0;
	
	std::ofstream configStream(configFile);
	
	if (configStream.is_open()) {
		std::map<std::string, std::string>::iterator it;
		
		for(it = configMap.begin(); it != configMap.end(); it++) {
			configStream << (*it).first << " : " << (*it).second << std::endl;
		}
		configStream.close();
	} else {
		ret = 1;
	}
	
	return ret;
}
	
std::string Config::getEntry(std::string entry) {
	std::string value = "";
	std::map<std::string, std::string>::iterator it;
	
	it = config->configMap.find(entry);
	
	if (it != config->configMap.end()) {
		value = (*it).second;
	}

	return value;
}

void Config::setEntry(std::string entry, std::string value) {
	config->configMap[entry] = value;
	save();
}


std::string Config::getSessionEntry(std::string entry) {
	std::string value = "";
	std::map<std::string, std::string>::iterator it;
	
	it = config->sessionMap.find(entry);
	
	if (it != config->sessionMap.end()) {
		value = (*it).second;
	}

	return value;
}

void Config::setSessionEntry(std::string entry, std::string value) {
	config->sessionMap[entry] = value;
}

void Config::printConfig() {
	std::map<std::string, std::string>::iterator it;
	
	for(it = config->configMap.begin(); it != config->configMap.end(); it++) {
		LogInfo << (*it).first << " : " << (*it).second;
	}
}
