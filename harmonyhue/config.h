#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

class Config {
public:
	static int init(std::string configFile) {
		if (!config) {
			config = new Config(configFile);
		}
		config->oLoad();
	}
	
	static void save() {
		if (config) {
			config->oSave();
		}
	}
		
	static void close() {
		if (config) {
			config->oSave();
			delete config;
			config = 0;
		}
	}
	
	static void change(std::string configFile) {
		Config* newConfig = new Config(configFile);
		
		std::map<std::string, std::string>::iterator it;
		
		for (it = config->configMap.begin(); it != config->configMap.end(); it++) {
			newConfig->configMap[(*it).first] = (*it).second;
		}
		for (it = config->sessionMap.begin(); it != config->sessionMap.end(); it++) {
			newConfig->sessionMap[(*it).first] = (*it).second;
		}
		
		close();
		
		config = newConfig;
		
		save();
	}
	
	int oLoad();
	int oSave();
	
	static std::string getEntry(std::string entry);
	static void setEntry(std::string entry, std::string value);
	
	static std::string getSessionEntry(std::string entry);
	static void setSessionEntry(std::string entry, std::string value);
	
	static void printConfig();
	

private:
	Config(std::string configFile) : configFile(configFile) {}
	
	static Config* config;
	std::string configFile;
	std::map<std::string, std::string> configMap;
	std::map<std::string, std::string> sessionMap;
};

#endif
