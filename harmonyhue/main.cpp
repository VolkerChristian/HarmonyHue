#include <algorithm>
#include <cstdlib>
#include <jansson.h>
#include <list>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "csocket.h"
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


int main(int argc, char* argv[]) {
	
	Logger::setToInfo();
	LogInfo << "LogiHue - Startup";
	
	Config::init("config");
	
	Config::printConfig();
	
//	Config::close();
	
//	exit(0);
	
	
    daemonize("logihue");
//	Logger::setToWarn();
    
    LogWarn << "Daemonized: Harmony-Hue";
	
    
    requestAuthToken();
    
	

	
//    exit(0);
    
    
    
    
	csocket harmonyCommunicationcsocket;
//	harmonyCommunicationcsocket.connect(strHarmonyIPAddress.c_str(), harmonyPortNumber);
	harmonyCommunicationcsocket.connect(Config::getEntry("HarmonyHub").c_str(), HARMONYPORT);
    
    
//    HARMONY::HParser* p = new HARMONY::HParser(&harmonyCommunicationcsocket);
	
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
