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
		}
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
