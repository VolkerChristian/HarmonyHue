#include <jansson.h>
#include <string.h>

#include "config.h"
#include "curl.h"
#include "hstates.h"
#include "logger.h"


namespace HARMONY {

#define HUE_URL "http://philips-hue.feuerweg.vchrist.at"
#define HUE_SENSOR "7"
#define HUE_WHITE "qjayddogNNF6X4C9IiVTjvd-XclsCXhT8sF1MyvL"


ERROR EventState::on_cdata_block (const Glib::ustring& text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text.c_str(), 0, &error);

    if(root) {
        json_t *activityStatus;
		json_t *runningActivityList;

        activityStatus = json_object_get(root, "activityStatus");
		runningActivityList = json_object_get(root, "runningActivityList");

        if(json_is_integer(activityStatus) && json_is_string(runningActivityList)) {
			if (std::string(json_string_value(runningActivityList)).empty()) {
				std::string response;
				switch (json_integer_value(activityStatus)) {
					case 0:
						EasyCurl::instance()->put("http://" + Config::getEntry("HueBridge") + 
																			   "/api/" + Config::getEntry("HueUserName") + 
																			   "/sensors/" + Config::getEntry("HueSensor") + 
																			   "/state", "{\"flag\" : false}", response, false);
						LogInfo << "Philips Hue: arm motion sensor and trigger auto turn off sequence: " << response;
						break;
					case 1:
						EasyCurl::instance()->put("http://" + Config::getEntry("HueBridge") + 
																			   "/api/" + Config::getEntry("HueUserName") + 
																			   "/sensors/" + Config::getEntry("HueSensor") + 
																			   "/state", "{\"flag\" : true}", response, false);
						LogInfo << "Philips Hue: disarm motion sensor and reset auto turn off sequence: " << response;
						break;
					case 2:
//					    EasyCurl::instance()->put("http://" + Config::getEntry("HueBridge") + 
//																		       "/api/" + Config::getEntry("HueUserName") + 
//																		       "/sensors/" + Config::getEntry("HueSensor") + 
//																		       "/state", "{\"flag\" : true}", response);
//					    LogInfo << "Philips Hue: disarm motion sensor and reset auto turn off sequence: " << response;
						break;
					case 3:
//					    EasyCurl::instance()->put("http://" + Config::getEntry("HueBridge") + 
//																		       "/api/" + Config::getEntry("HueUserName") + 
//																		       "/sensors/" + Config::getEntry("HueSensor") + 
//																		       "/state", "{\"flag\" : true}", response);
//					    LogInfo << "Philips Hue: disarm motion sensor and reset auto turn off sequence: " << response;
						break;
				}
			}
        }
    }

    json_decref(root);
    
    return ERROR::E_SUCCESS;
}

ERROR OaState::on_start_element(const Glib::ustring& name,
										 const xmlpp::SaxParser::AttributeList& attributes) {
	isPing = false;
	isPair = false;
	
	
	for(const auto& attr_pair : attributes) {
		if (attr_pair.name == "mime") {
			isPair = attr_pair.value == "vnd.logitech.connect/vnd.logitech.pair";
			isPing = attr_pair.value == "vnd.logitech.connect/vnd.logitech.ping";
		}
    }
	
	return ERROR::E_SUCCESS;
}

ERROR OaState::on_cdata_block (const Glib::ustring& text) {
	ERROR error = ERROR::E_SUCCESS;
	
	if (isPair) {
		std::string identityTokenTag = "identity=";
		
		size_t pos = text.find(identityTokenTag);
		if (pos == std::string::npos) {
			error = ERROR::E_AUTH_NO_SESSION_TOKEN;
		} else {
			std::string sessionToken = text.substr(pos + identityTokenTag.length());
			
			pos = sessionToken.find(":status=succeeded");
			if (pos == std::string::npos) {
				error = ERROR::E_AUTH_NOT_SUCCEEDED;
			} else {
				sessionToken = sessionToken.substr(0, pos);
				Config::setSessionEntry("SessionToken", sessionToken);
				LogInfo << "Got session token: " << sessionToken;
			}
		}
	} else if (isPing) {
		LogDebug << "Hub is alive " << Config::getSessionEntry("SessionToken");
	}
	
	return error;
}

}
