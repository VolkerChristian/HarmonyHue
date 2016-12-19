#include "hparser.h"

#include "csocket.h"
#include "logger.h"

namespace HARMONY {

void HParser::on_start_document() {
	p_on_start_document();
    error = hContext->on_start_document();
}

void HParser::on_end_document() {
	p_on_end_document();
    error = hContext->on_end_document();
}

void HParser::on_start_element(const Glib::ustring& name,
                               const AttributeList& attributes) {
	p_on_start_element(name, attributes);
    error = hContext->on_start_element(name, attributes);
}

void HParser::on_end_element(const Glib::ustring& name) {
	p_on_end_element(name);
    error = hContext->on_end_element(name);
}

void HParser::on_characters(const Glib::ustring& characters) {
	p_on_characters(characters);
    error = hContext->on_characters(characters);
}

void HParser::on_comment(const Glib::ustring& text) {
	p_on_comment(text);
    error = hContext->on_comment(text);
}

void HParser::on_warning(const Glib::ustring& text) {
	p_on_warning(text);
    error = hContext->on_warning(text);
}

void HParser::on_error(const Glib::ustring& text) {
	p_on_error(text);
    error = hContext->on_error(text);
}

void HParser::on_fatal_error(const Glib::ustring& text) {
	p_on_fatal_error(text);
    error = hContext->on_fatal_error(text);
}

void HParser::on_cdata_block (const Glib::ustring& text) {
	p_on_cdata_block(text);
    error = hContext->on_cdata_block(text);
}

ERROR HParser::hParse() {
	error = ERROR::E_SUCCESS;
	
    try {
        char buffer[64];
        const size_t buffer_size = sizeof(buffer) / sizeof(char);
        int bytesRead = 0;
		bool canRead = false;
		float remainingSecs = 30;
		bool heardBeat = false;
		
        do {
			heardBeat = false;
			if (cSocket->canRead(&canRead, &remainingSecs, remainingSecs) == FAILURE) {
				error = ERROR::E_CANT_READ_FROM_SOCKET;
				break;
			}
            if (canRead) {
                std::memset(buffer, 0, buffer_size);
                bytesRead = cSocket->read(buffer, buffer_size-1, false);
                if(bytesRead > 0) {
                    // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
                    // instead of Glib::ustring::ustring( const char*, size_type ) because it
                    // expects the length of the string in characters, not in bytes.
                    Glib::ustring input(buffer, buffer + bytesRead);
                    parse_chunk(input);
                }
            } else {
				LogDebug << "Heardbeat";
                cSocket->write("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>", strlen("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>"));
				remainingSecs = 30;
				heardBeat = true;
            }
        }  while(((bytesRead > 0 || errno == EINTR) && error == ERROR::E_SUCCESS) || heardBeat);
		finish_chunk_parsing();
    }
    catch(const xmlpp::exception& ex) {
        LogError << "Incremental parsing, libxml++ exception: " << ex.what();
        error = ERROR::E_XML_PARSER_EXCEPTION;
    }

    return error;
}


ERROR HContext::on_start_document() {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_start_document();
    } else {
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_end_document() {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_end_document();
    } else {
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_start_element(const Glib::ustring& name,
                                      const xmlpp::SaxParser::AttributeList& attributes) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        HState* hsNext = hsCurrent->getHsChild(name);
        if (hsNext) {
            error = (hsCurrent = hsNext)->on_start_element(name, attributes);
        } else {
            // ERROR no next state found
            error = ERROR::E_NO_NEXT_STATE_ERROR;
        }
    } else {
        // ERROR no current state
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_end_element(const Glib::ustring& name) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        if(hsCurrent->getHsName() == name) {
            error = hsCurrent->on_end_element(name);
			hsCurrent->write();
            hsCurrent = hsCurrent->getHsParent();
            if (!hsCurrent->getHsParent()) {
                error = ERROR::E_END_DOCUMENT;
            }
        } else {
            // ERROR current state and requested state to end do not match
            error = ERROR::E_WRONG_END_ELEMENT;
        }
    } else {
        // ERROR no current state to end
        error = ERROR::E_NO_CURRENT_STATE_TO_END;
    }

    return error;
}

ERROR HContext::on_characters(const Glib::ustring& characters) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_characters(characters);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_comment(const Glib::ustring& text) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_comment(text);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_warning(const Glib::ustring& text) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_warning(text);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_error(const Glib::ustring& text) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_error(text);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_fatal_error(const Glib::ustring& text) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_fatal_error(text);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

ERROR HContext::on_cdata_block (const Glib::ustring& text) {
    ERROR error = ERROR::E_SUCCESS;

    if (hsCurrent) {
        error = hsCurrent->on_cdata_block(text);
    } else {
        // ERROR
        error = ERROR::E_NO_CURRENT_STATE;
    }

    return error;
}

// xml-events
ERROR HStateAdapter::on_start_document() {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_end_document() {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_start_element(const Glib::ustring& name,
        const xmlpp::SaxParser::AttributeList& attributes) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_end_element(const Glib::ustring& name) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_characters(const Glib::ustring& characters) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_comment(const Glib::ustring& text) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_warning(const Glib::ustring& text) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_error(const Glib::ustring& text) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_fatal_error(const Glib::ustring& text) {
    return ERROR::E_SUCCESS;
}

ERROR HStateAdapter::on_cdata_block (const Glib::ustring& text) {
    return ERROR::E_SUCCESS;
}

} // END namespace
