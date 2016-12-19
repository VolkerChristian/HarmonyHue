#include <algorithm>
#include <cstdlib>
#include <jansson.h>
#include <list>

#include <argp.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "authorization.h"
#include "config.h"
#include "curl.h"
#include "logger.h"

#include "hparser.h"
#include "hstates.h"

#include "myxmltextreader.h"

static sighandler_t handle_signal (int sig_nr, sighandler_t signalhandler) {
    struct sigaction neu_sig, alt_sig;
    neu_sig.sa_handler = signalhandler;
    sigemptyset (&neu_sig.sa_mask);
    neu_sig.sa_flags = SA_RESTART;
    if (sigaction (sig_nr, &neu_sig, &alt_sig) < 0)
        return SIG_ERR;
    return alt_sig.sa_handler;
}

static void daemonize(const char* logName) {
    int i;
    pid_t pid;
	
	LogInfo << "Start daemonizing";
    /* Elternprozess beenden, somit haben wir einen Waisen */
    /* dessen sich jetzt vorerst init annimmt              */
    if ((pid = fork ()) != 0) {
        exit (EXIT_SUCCESS);
    }
    /* Kindprozess wird zum Sessionführer. Misslingt */
    /* dies, kann der Fehler daran liegen, dass der     */
    /* Prozess bereits Sessionführer ist               */
    if (setsid() < 0) {
        LogFatal << " kann nicht Sessionführer werden!";
        exit (EXIT_FAILURE);
    }
    /* Signal SIGHUP ignorieren */
    handle_signal (SIGHUP, SIG_IGN);
    /* Oder einfach: signal(SIGHUP, SIG_IGN) ... */
    /* Das Kind terminieren */
    if ((pid = fork ()) != 0) {
        exit (EXIT_SUCCESS);
    }
    /* Gründe für das Arbeitsverzeichnis:
     * + core-Datei wird im aktuellen Arbeitsverzeichnis
     *   hinterlegt.
     * + Damit bei Beendigung mit umount das Dateisystem
     *   sicher abgehängt werden kann
     */
    chdir ("/");
    /* Damit wir nicht die Bitmaske vom Elternprozess     */
    /* erben bzw. diese bleibt, stellen wir diese auf 0  */
    umask (0);
    /* Wir schließen alle geöffneten Filedeskriptoren ... */
    for (i = sysconf (_SC_OPEN_MAX); i > 0; i--)
        close (i);
	Logger::daemonized();
}

const char* argp_program_version = "HarmonyHue 0.8";

static char doc[] = "HarmonyHue reacts on state-changes of the harmony hub and sets a flag in the Philips Hue bridge";

static char args_doc[] = "";

static struct argp_option options[] = {
	{"configfile",  'f', "FILE",     0, "Path to a config file", 0},
	{"nodaemon",    'n', 0,          0, "Do not fork into the background", 1},
	{"loglevel",    'l', "LEVEL",    0, "Specify the loglevel", 2},
	{0,              0,  0,          0, "Logitech Harmony options"},
	{"email",       'e', "EMAIL",    0, "E-Mail address of your logitech account", 3},
	{"password",    'p', "PASSWORD", 0, "Password of your logitech account", 3},
	{"harmonyhub",  'h', "HOSTNAME", 0, "Hostname or IP-address of your harmony hub", 3},
	{0,              0,  0,          0, "Philips Hue options"},
	{"huebridge",   'b', "HOSTNAME", 0, "Hostname or IP-address of your hue bridge", 4},
	{"huesensor",   's', "SENSORID", 0, "Sensor id used to arm/disarm the dim-sequence", 4},
	{"hueusername", 'u', "USERSTR",  0, "String of the white-list of the hue bridge", 4},
	{0}
};

struct arguments {
	int logLevel;
	bool nodaemon;
	std::string configFile;
	std::string email;
	std::string password;
	std::string harmonyHub;
	std::string hueBridge;
	std::string sensor;
	std::string hueUserName;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
	struct arguments* arguments = (struct arguments*) state->input;
	
	switch(key) {
		case 'f':
			Config::change(arg);
			break;
		case 'l':
			arguments->logLevel = atoi(arg);
			break;
		case 'n':
			arguments->nodaemon = true;
			break;
		case 'e':
			arguments->email = arg;
			break;
		case 'p':
			arguments->password = arg;
			break;
		case 'h':
			arguments->harmonyHub = arg;
			break;
		case 'b':
			arguments->hueBridge = arg;
			break;
		case 's':
			arguments->sensor = arg;
			break;
		case 'u':
			arguments->hueUserName = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
};

int main(int argc, char* argv[]) {
	Logger::setToInfo();
	
	LogInfo << "ConfigFile: " << CONFIG_FILE;
	
	Config::init(CONFIG_FILE);
	
	struct arguments arguments;
	
	arguments.nodaemon = false;
	arguments.logLevel = 5;
	arguments.email = Config::getEntry("HarmonyEmail");
	arguments.password = Config::getEntry("HarmonyPassword");
	arguments.harmonyHub = Config::getEntry("HarmonyHub");
	arguments.hueBridge = Config::getEntry("HueBridge");
	arguments.sensor = Config::getEntry("HueSensor");
	arguments.hueUserName = Config::getEntry("HueUserName");
	
	struct argp argp = {options, parse_opt, args_doc, doc};
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	
	Config::setEntry("HarmonyEmail", arguments.email);
	Config::setEntry("HarmonyPassword", arguments.password);
	Config::setEntry("HarmonyHub", arguments.harmonyHub);
	Config::setEntry("HueBridge", arguments.hueBridge);
	Config::setEntry("HueSensor", arguments.sensor);
	Config::setEntry("HueUserName", arguments.hueUserName);
	
	Config::printConfig();
	
	switch(arguments.logLevel) {
		case 0:
			Logger::setToFatal();
			break;
		case 1:
			Logger::setToCrit();
			break;
		case 2:
			Logger::setToError();
			break;
		case 3:
			Logger::setToWarn();
			break;
		case 4:
			Logger::setToNotice();
			break;
		case 5:
			Logger::setToInfo();
			break;
		case 6:
			Logger::setToDebug();
			break;
		default:
			LogError << "Wrong log-level " << arguments.logLevel;
			exit(1);
	};
	
	LogInfo << "LogiHue - Startup";

	if (!arguments.nodaemon) {
		daemonize("logihue");
		LogInfo << "Daemonized: Harmony-Hue";
	}
	
	while (true) {
		LogInfo << "Trying to connect to Harmony-Hub";
		connectToHubAndParse();
		LogError << "Disconnected from Harmony-Hub";
		sleep(5);
	}
	
	return EXIT_SUCCESS;
}
