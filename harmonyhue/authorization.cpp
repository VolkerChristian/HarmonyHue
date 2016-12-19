#include "authorization.h"

#include <algorithm>
#include <jansson.h>

#include "base64.h"
#include "curl.h"

#define CONNECTION_ID "12345678-1234-5678-1234-123456789012-1"

HARMONY::ERROR authorizeWithAuthorizationToken() { // on success return a non-empty session token
	HARMONY::HParser* parser = new HARMONY::HParser();

    HARMONY::HWriter* writer = parser->getWriter("<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>");
	HARMONY::DocumentState* document = new HARMONY::DocumentState(writer);
    
    HARMONY::ConnectState* connect = new HARMONY::ConnectState();
    
    writer = parser->getWriter("</stream:stream>");
    HARMONY::ConnectError* connectError = new HARMONY::ConnectError(writer);
    
	std::string data = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">";
    std::string tmp = "\0";
    tmp.append("guest");
    tmp.append("\0");
    tmp.append("gatorade.");
    data.append(base64_encode(tmp.c_str(), tmp.length()));
    data.append("</auth>");
	writer = parser->getWriter(data);
    HARMONY::StreamFeaturesState* streamFeature = new HARMONY::StreamFeaturesState(writer);
    
    HARMONY::MechanismsState* mechanisms = new HARMONY::MechanismsState();
    
    HARMONY::MechanismState* mechanism = new HARMONY::MechanismState();
    
	writer = parser->getWriter("<iq type=\"get\" id=\"" + std::string(CONNECTION_ID) + "\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.connect/vnd.logitech.pair\">token=" + Config::getEntry("HarmonyAuthToken") + ":name=foo#iOS6.0.1#iPhone</oa></iq>");
	HARMONY::SuccessState* success = new HARMONY::SuccessState(writer);
    
    writer = parser->getWriter("</stream:stream>"); // we break parsing after login test
    HARMONY::IqState* iq = new HARMONY::IqState(writer);
    
    HARMONY::OaState* oa = new HARMONY::OaState();
	
	document->addHsState(connect);
	connect->addHsState(streamFeature);
	streamFeature->addHsState(mechanisms);
	mechanisms->addHsState(mechanism);
	connect->addHsState(success);
	connect->addHsState(iq);
	iq->addHsState(oa);
	document->addHsState(connectError);
	
	HARMONY::ERROR error = HARMONY::ERROR::E_SUCCESS;
	
	parser->getHContext()->setInitialHsState(document);
	
	error = parser->connect();
	if (error == HARMONY::ERROR::E_SUCCESS) {
		error = parser->hParse();
	}
	
	delete parser;
	delete document;
	delete connect;
	delete connectError;
	delete streamFeature;
	delete mechanisms;
	delete mechanism;
	delete success;
	delete iq;
	delete oa;
	
	return error;
}

HARMONY::ERROR retriveAuthorizationTokenFromLogitech() { // on success return a non-empty authorization token
	LogInfo << "Querry Logitech for AuthentificationToken";
	
	HARMONY::ERROR error = HARMONY::ERROR::E_SUCCESS;
	
	std::string logitechUrl("http://svcs.myharmony.com/CompositeSecurityServices/Security.svc/json/GetUserAuthToken");
	std::string content("{\"email\":\"" + Config::getEntry("HarmonyEmail") + "\",\"password\":\"" + Config::getEntry("HarmonyPassword") + "\"}");

	std::list<std::string> header;
	header.push_back("Accept-Encoding: identity");
	header.push_back("content-type: application/json;charset=utf-8");
	header.push_back("Content-Length: " + std::to_string(content.length()));
	
	std::string response;
	if (EasyCurl::instance()->post(logitechUrl, content, header, response) == HARMONY::ERROR::E_SUCCESS) {
		LogInfo << "Response from Logitech: " << response;
		
		json_t *root;
		json_error_t json_error;
		
		root = json_loads(response.c_str(), 0, &json_error);
		
		if (root) {
			if (json_is_object(root)) {
				json_t* userAuthTokenResult = json_object_get(root, "GetUserAuthTokenResult");
				if (json_is_object(userAuthTokenResult)) {
					json_t* userAuthToken = json_object_get(userAuthTokenResult, "UserAuthToken");
					if (json_is_string(userAuthToken)) {
						std::string authToken = json_string_value(userAuthToken);
						authToken.erase(std::remove(authToken.begin(), authToken.end(), '\\'), authToken.end());
						Config::setEntry("HarmonyAuthToken", authToken);
						LogInfo << "AuthorizationToken = " << authToken;
					} else {
						error = HARMONY::ERROR::E_JSON;
					}
				} else {
					error = HARMONY::ERROR::E_JSON;
				}
			} else {
				error = HARMONY::ERROR::E_JSON;
			}
		} else {
			error = HARMONY::ERROR::E_JSON;
		}
	} else {
		error == HARMONY::ERROR::E_CURL;
	}
	
	return error;
}

HARMONY::ERROR loginWithSessionToken() { // on success return a valid parser-object
	HARMONY::HParser* parser = new HARMONY::HParser();

    HARMONY::HWriter* writer = parser->getWriter("<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>");
	HARMONY::DocumentState* document = new HARMONY::DocumentState(writer);
    
    HARMONY::ConnectState* connect = new HARMONY::ConnectState();
    
    writer = parser->getWriter("</stream:stream>");
    HARMONY::ConnectError* connectError = new HARMONY::ConnectError(writer);
    
	std::string data = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">";
    std::string tmp = "\0";
    tmp.append(Config::getSessionEntry("SessionToken"));
    tmp.append("\0");
    tmp.append(Config::getSessionEntry("SessionToken"));
    data.append(base64_encode(tmp.c_str(), tmp.length()));
    data.append("</auth>");
	writer = parser->getWriter(data);
    HARMONY::StreamFeaturesState* streamFeature = new HARMONY::StreamFeaturesState(writer);
    
    HARMONY::MechanismsState* mechanisms = new HARMONY::MechanismsState();
    
    HARMONY::MechanismState* mechanism = new HARMONY::MechanismState();
    
	HARMONY::SuccessState* success = new HARMONY::SuccessState();
    
    HARMONY::IqState* iq = new HARMONY::IqState();
    
    HARMONY::OaState* oa = new HARMONY::OaState();
	
    HARMONY::MessageState* message = new HARMONY::MessageState();
    
    HARMONY::EventState* event = new HARMONY::EventState();
	
	document->addHsState(connect);
	connect->addHsState(streamFeature);
	streamFeature->addHsState(mechanisms);
	mechanisms->addHsState(mechanism);
	connect->addHsState(success);
	connect->addHsState(iq);
	iq->addHsState(oa);
	connect->addHsState(message);
	message->addHsState(event);
	document->addHsState(connectError);
	
	HARMONY::ERROR error = HARMONY::ERROR::E_SUCCESS;
	
	parser->getHContext()->setInitialHsState(document);
	
	error = parser->connect();
	if (error == HARMONY::ERROR::E_SUCCESS) {
		error = parser->hParse();
	}
	
	delete parser;
	delete document;
	delete connect;
	delete connectError;
	delete streamFeature;
	delete mechanisms;
	delete mechanism;
	delete success;
	delete iq;
	delete oa;
	delete message;
	delete event;
	
	return error;
}

HARMONY::ERROR connectToHubAndParse() { // use all functions above and return a valid parser-object on success
	HARMONY::ERROR error = HARMONY::ERROR::E_SUCCESS;
	
	if (Config::getEntry("HarmonyAuthToken").empty()) {
		error = retriveAuthorizationTokenFromLogitech();
	}
	if (error == HARMONY::ERROR::E_SUCCESS) {
		error = authorizeWithAuthorizationToken();
		if (!(error == HARMONY::ERROR::E_SUCCESS)) {
			error = retriveAuthorizationTokenFromLogitech();
			if (error == HARMONY::ERROR::E_SUCCESS) {
				error = authorizeWithAuthorizationToken();
			}
		}
		if (error == HARMONY::ERROR::E_SUCCESS) {
			error = loginWithSessionToken();
		}
	}
	
	return error;
}
