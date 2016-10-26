#include <cstdlib>
#include <list>

#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "csocket.h"
#include "curl.h"
#include "harmony.h"
#include "logger.h"

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
    if ((pid = fork ()) != 0)
        exit (EXIT_FAILURE);
    /* Kindprozess wird zum Sessionführer. Misslingt */
    /* dies, kann der Fehler daran liegen, dass der     */
    /* Prozess bereits Sessionführer ist               */
    if (setsid() < 0) {
        Logger::error << " kann nicht Sessionführer werden!";
        exit (EXIT_FAILURE);
    }
    /* Signal SIGHUP ignorieren */
    handle_signal (SIGHUP, SIG_IGN);
    /* Oder einfach: signal(SIGHUP, SIG_IGN) ... */
    /* Das Kind terminieren */
    if ((pid = fork ()) != 0)
        exit (EXIT_FAILURE);
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

int main(int argc, char* argv[]) {
	Logger::setToInfo();
	Logger::info << "LogiHue - Startup";
//    daemonize("logihue");
	Logger::setToWarn();
    Logger::warn << "Daemonized: Harmony-Hue";
	
	
	std::string strHarmonyIPAddress = "harmonyhub.feuerweg.vchrist.at";
	int harmonyPortNumber = 5222;
	
	std::string logitechUrl("http://svcs.myharmony.com/CompositeSecurityServices/Security.svc/json/GetUserAuthToken");
	std::string content("{\"email\":\"volker.christian@liwest.at\",\"password\":\"pentium5\"}");

	std::list<std::string> header;
	header.push_back("Accept-Encoding: identity");
	header.push_back("content-type: application/json;charset=utf-8");
	header.push_back("Content-Length: " + std::to_string(content.length()));
	
	std::string response;
	
	EasyCurl::instance()->post(logitechUrl, content, header, response);
	
	std::cout << "Response: " << response << std::endl;
	
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
	
    return EXIT_SUCCESS;
}
