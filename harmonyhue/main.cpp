#include <algorithm>
#include <cstdlib>
#include <jansson.h>
#include <list>

#include <argp.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "curl.h"
#include "harmony.h"
#include "logger.h"

#include "hparser.h"
#include "hstates.h"

#define HARMONYPORT 5222
#define CONNECTION_ID "12345678-1234-5678-1234-123456789012-1"

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
    /* Elternprozess beenden, somit haben wir einen Waisen */
    /* dessen sich jetzt vorerst init annimmt              */
    if ((pid = fork ()) != 0) {
        exit (EXIT_FAILURE);
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
        exit (EXIT_FAILURE);
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


int requestAuthToken() {
    LogInfo << "Querry Logitech for AuthentificationToken";
	
	std::string logitechUrl("http://svcs.myharmony.com/CompositeSecurityServices/Security.svc/json/GetUserAuthToken");
	std::string content("{\"email\":\"" + Config::getEntry("HarmonyEmail") + "\",\"password\":\"" + Config::getEntry("HarmonyPassword") + "\"}");

	std::list<std::string> header;
	header.push_back("Accept-Encoding: identity");
	header.push_back("content-type: application/json;charset=utf-8");
	header.push_back("Content-Length: " + std::to_string(content.length()));
	
	std::string response;
	EasyCurl::instance()->post(logitechUrl, content, header, response);
	LogInfo << "Response from Logitech: " << response;
	
    json_t *root;
    json_error_t error;
	
    root = json_loads(response.c_str(), 0, &error);
	if (root && json_is_object(root)) {
		json_t* userAuthTokenResult = json_object_get(root, "GetUserAuthTokenResult");
		if (json_is_object(userAuthTokenResult)) {
			json_t* userAuthToken = json_object_get(userAuthTokenResult, "UserAuthToken");
			if (json_is_string(userAuthToken)) {
				std::string authToken = json_string_value(userAuthToken);
				authToken.erase(std::remove(authToken.begin(), authToken.end(), '\\'), authToken.end());
				Config::setEntry("HarmonyAuthToken", authToken);
				LogInfo << "AuthorizationToken = " << authToken;
			}
		}
	}
	
	return 0;
}


const char* argp_program_version = "HarmonyHue 0.8";

static char doc[] = "HarmonyHue reacts on state-changes of the harmony hub and sets a flag in the Philips Hue bridge";

static char args_doc[] = "ARG1 [STRING...]";

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
			arguments->configFile = arg;
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

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
	Logger::setToInfo();
	
	Config::init("config");
	
	struct arguments arguments;
	
	arguments.nodaemon = false;
	arguments.logLevel = 5;
	arguments.email = Config::getEntry("HarmonyEmail");
	arguments.password = Config::getEntry("HarmonyPassword");
	arguments.harmonyHub = Config::getEntry("HarmonyHub");
	arguments.hueBridge = Config::getEntry("HueBridge");
	arguments.sensor = Config::getEntry("HueSensor");
	arguments.hueUserName = Config::getEntry("HueUserName");
	
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
			LogInfo << "Wrong log-level " << arguments.logLevel;
			exit(1);
	};
	
	LogInfo << "LogiHue - Startup";
	
	if (!arguments.nodaemon) {
		daemonize("logihue");
		LogInfo << "Daemonized: Harmony-Hue";
	}

    requestAuthToken();
	
	/* Connect to harmony hub with guest user
		* try the authorization token
		* if ok
			* receive the session token
		* if !ok
			* break parsing
			* close socket to harmony hub
			* connect to logitech
			* receive the authorization token
			* close socket to logitech
			* connect to harmony hub with guest user
				* try the new authorization token
				* if ok
					* receive the session token
				* if !ok
					* exit
		* close the socket to harmony hub
		* connect to harmony hub with session token as user and password
						* maybe it is enough to authenticate again with session token as user and password
		* start parsing
	end connect to harmony hub */
	
	HARMONY::HParser* p = new HARMONY::HParser();
	p->connect();
    
    HARMONY::HWriter* writer = p->getWriter("<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>");
	
    
    HARMONY::DocumentState* ds = new HARMONY::DocumentState(writer);
    
    HARMONY::ConnectState* cs = new HARMONY::ConnectState();
    
    writer = p->getWriter("</stream:stream>");
    
    HARMONY::ConnectError* ce = new HARMONY::ConnectError(writer);
    
	std::string data = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">";
    std::string tmp = "\0";
    tmp.append("guest");
    tmp.append("\0");
    tmp.append("gatorade.");
    data.append(base64_encode(tmp.c_str(), tmp.length()));
    data.append("</auth>");
	writer = p->getWriter(data);
	
    HARMONY::StreamFeaturesState* sfs = new HARMONY::StreamFeaturesState(writer);
    
    HARMONY::MechanismsState* mss = new HARMONY::MechanismsState();
    
    HARMONY::MechanismState* ms = new HARMONY::MechanismState();
    
	writer = p->getWriter("<iq type=\"get\" id=\"" + std::string(CONNECTION_ID) + "\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.connect/vnd.logitech.pair\">token=" + Config::getEntry("HarmonyAuthToken") + ":name=foo#iOS6.0.1#iPhone</oa></iq>");
		
    HARMONY::SuccessState* ss = new HARMONY::SuccessState(writer);
    
    writer = p->getWriter("</stream:stream>");
    
    HARMONY::IqState* iq = new HARMONY::IqState(); //writer);
    
    HARMONY::OaSwapTokenState* oa = new HARMONY::OaSwapTokenState();
    
    ds->addHsState(cs);
    
    cs->addHsState(sfs);
    
    sfs->addHsState(mss);
    
    mss->addHsState(ms);
    
    cs->addHsState(ss);
    
    cs->addHsState(iq);
    
    iq->addHsState(oa);
    
    HARMONY::MessageState* message = new HARMONY::MessageState();
    
    HARMONY::EventState* event = new HARMONY::EventState();
    
    message->addHsState(event);
    
    cs->addHsState(message);
    
    p->getHContext()->setInitialHsState(ds);
    
    p->hParse();
    
    
    /*
    harmonyCommunicationcsocket.close();
	harmonyCommunicationcsocket.connect(strHarmonyIPAddress.c_str(), harmonyPortNumber);
    p->getHContext()->setInitialHsState(ds);
    
    p->hParse();
    */
    
    
    
    
    
    
    
    
    
    
    
    
    
/*    
	csocket harmonyCommunicationcsocket;
	harmonyCommunicationcsocket.connect(strHarmonyIPAddress.c_str(), harmonyPortNumber);
	
	HarmonyParser parser(&harmonyCommunicationcsocket);
	
	HarmonyParserAdapter* connect = new ConnectAdapter();
	if (!parser.startParse(connect) != HarmonyParser::State::next) {
	}
	
	HarmonyParserAdapter* login = new LoginAdapter("guest", "gatorade.");
	if (!parser.startParse(login) != HarmonyParser::State::next) {
	}
	
	std::string authorizationToken = "n/LCkMcFkXZVc5UhbjT6+1xdX0/2fhC3Kr17x2SrhDRX+x+9dqcjLQZz/F3vkm7E";
	AuthorizationTokenAdapter* authorization = new AuthorizationTokenAdapter(authorizationToken);
	if (!parser.startParse(authorization) != HarmonyParser::State::next) {
	}

	std::cout << "Token: " << authorizationToken << std::endl;
	
	harmonyCommunicationcsocket.close();
	
	harmonyCommunicationcsocket.connect(strHarmonyIPAddress.c_str(), harmonyPortNumber);
	
	HarmonyParser parser2(&harmonyCommunicationcsocket);
	if (!parser2.startParse(connect) != HarmonyParser::State::next) {
	}
	
	HarmonyParserAdapter* login2 = new LoginAdapter(authorizationToken, authorizationToken);
	if (!parser2.startParse(login) != HarmonyParser::State::next) {
	}
	
	HarmonyParserAdapter* adapter = new HarmonyParserAdapter();
	if (!parser2.startParse(adapter) != HarmonyParser::State::next) {
	}
*/
    return EXIT_SUCCESS;
}
