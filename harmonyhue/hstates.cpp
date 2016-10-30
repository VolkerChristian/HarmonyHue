#include <jansson.h>
#include <string.h>

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

        activityStatus = json_object_get(root, "activityStatus");

        if(json_is_integer(activityStatus)) {
            std::string response;
            switch (json_integer_value(activityStatus)) {
                case 0:
					LogInfo << "Philips Hue: arm motion sensor and trigger auto turn off timer";
                    EasyCurl::instance()->put(HUE_URL "/api/" HUE_WHITE "/sensors/" HUE_SENSOR "/state", "{\"flag\" : false}", response);
                    break;
                case 1:
					LogInfo << "Philips Hue: unarm motion sensor and reset auto turn off sequence";
                    EasyCurl::instance()->put(HUE_URL "/api/" HUE_WHITE "/sensors/" HUE_SENSOR "/state", "{\"flag\" : true}", response);
                    break;
                case 2:
//					LogInfo << "Philips Hue: unarm motion sensor and reset auto turn off sequence";
//                    EasyCurl::instance()->put(HUE_URL "/api/" HUE_WHITE "/sensors/" HUE_SENSOR "/state", "{\"flag\" : true}", response);
                    break;
                case 3:
//					LogInfo << "Philips Hue: unarm motion sensor and reset auto turn off sequence";
//                    EasyCurl::instance()->put(HUE_URL "/api/" HUE_WHITE "/sensors/" HUE_SENSOR "/state", "{\"flag\" : true}", response);
                    break;
            }
        }
    }

    json_decref(root);
    
    return ERROR::E_SUCCESS;
}

}
