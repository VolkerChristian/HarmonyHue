#ifndef __HARMONYPARSER_H__
#define __HARMONYPARSER_H__

#include <iostream>
#include <list>
#include <map>
#include <stack>

#include <jansson.h>
#include <libxml++/libxml++.h>

#include "base64.h"
#include "config.h"
#include "csocket.h"
#include "logger.h"


#define HARMONYPORT 5222

namespace HARMONY {

enum ERROR {
    E_SUCCESS = 0,
    E_CONTINUE,
	E_BREAK,
    E_END_DOCUMENT,
    E_NO_NEXT_STATE_ERROR,
    E_NO_CURRENT_STATE,
    E_WRONG_END_ELEMENT,
    E_NO_CURRENT_STATE_TO_END,
    E_CANT_READ_FROM_SOCKET,
    E_XML_PARSER_EXCEPTION,
	E_AUTH_NO_SESSION_TOKEN,
	E_AUTH_NOT_SUCCEEDED,
	E_NOT_CONNECTED,
	E_WRITE,
	E_JSON,
	E_CURL
};


class HContext;
class HParser;
class HState;

class HWriter {
public:
	void setText(std::string text) {
		this->text = text;
	}
	
private:
	HWriter(csocket* cSocket, std::string text) : cSocket(cSocket), text(text) {}

	ERROR write() {
		LogDebug << "Writing to harmony hub: " << text;
		
		ERROR error = ERROR::E_SUCCESS;
		
		if (cSocket->write(text.c_str(), text.length()) != text.length()) {
			error = ERROR::E_WRITE;
		}
		
		if (cSocket->getState() == csocket::SocketState::ERRORED) {
			error = ERROR::E_WRITE;
		}
		return error;
	}
	
	csocket* cSocket;
	std::string text;
	
	friend HParser;
	friend HState;
};

class XmlStanza {
public:
	XmlStanza(csocket* cSocket) : cSocket(cSocket) {}
	
public:
	std::string buildStanza() {
		std::string stanza = "<";
		
		stanza.append(name);
		
		if (!attrList.empty()) {
			std::list<Attribute>::iterator it;
			for (it = attrList.begin(); it != attrList.end(); it++) {
				stanza.append(" " + (*it).name() + "=" + (*it).value());
			}
		}
		
		if (!text.empty()) {
			stanza.append(">" + text + "</" + name + ">");
		} else if (!subStanza.empty()) {
			stanza.append(">\n");
			
			std::list<XmlStanza>::iterator it;
			for (it = subStanza.begin(); it != subStanza.end(); it++) {
				stanza.append("\t" + (*it).buildStanza() + "\n");
			}
			
			stanza.append("</" + name + ">");
		} else {
			stanza.append("/>");
		}
		
		return stanza;
	}
	
	void setName(std::string name) {
		this->name = name;
	}
		
	void addAttribute(std::string name, std::string value) {
		attrList.push_back(Attribute(name, value));
	}
		
	void setText(std::string text) {
		this->text = text;
	}
	
	void addSubStanza(XmlStanza stanza) {
		subStanza.push_back(stanza);
	}
	
private:
	class Attribute {
	public:
		Attribute(std::string name, std::string value) : xmlName(name), xmlValue(value) {}
		
		std::string name() {
			return xmlName;
		}
		
		std::string value() {
			return xmlValue;
		}
	
	private:
		std::string xmlName;
		std::string xmlValue;
	};
	
	csocket* cSocket;
	std::string name;
	std::string text;
	std::list<Attribute> attrList;
	std::list<XmlStanza> subStanza;
};


class HState {
public:
    HState(std::string hsName, HWriter* hWriter = 0) : hsName(hsName), hsParent(0), hWriter(hWriter) {
        // in concrete states set the name of the state
    }

    virtual ~HState() {}

    std::string getHsName() {
        return hsName;
    }

    void addHsState(HState* hState) {
        hState->hsParent = this;
        hsMap[hState->getHsName()] = hState;
    }

    HState* getHsChild(std::string name) {
		HState* rHState = 0;
		
		std::map<std::string, HState*>::iterator it = hsMap.find(name);
		if (it != hsMap.end()) {
			rHState = (*it).second;
		}
		return rHState;
    }

    HState* getHsParent() {
        return hsParent;
    }
    
	ERROR write() {
		ERROR error = ERROR::E_SUCCESS;
		if (hWriter != 0) {
			error = hWriter->write();
		}
		return error;
	}
    
protected:
    // state change notifications
    virtual ERROR on_reactivate(std::string deactivatedChild) = 0;
    virtual ERROR on_activate_error(std::string next) = 0;

    // xml-events
    virtual ERROR on_start_document() = 0;
    virtual ERROR on_end_document() = 0;
    virtual ERROR on_start_element(const Glib::ustring& name,
                                   const xmlpp::SaxParser::AttributeList& attributes) = 0;
    virtual ERROR on_end_element(const Glib::ustring& name) = 0;
    virtual ERROR on_characters(const Glib::ustring& characters) = 0;
    virtual ERROR on_comment(const Glib::ustring& text) = 0;
    virtual ERROR on_warning(const Glib::ustring& text) = 0;
    virtual ERROR on_error(const Glib::ustring& text) = 0;
    virtual ERROR on_fatal_error(const Glib::ustring& text) = 0;
    virtual ERROR on_cdata_block (const Glib::ustring& text) = 0;

private:
    std::string hsName;
    HState* hsParent;
	HWriter* hWriter;
    std::map<std::string, HState*> hsMap;
	
	friend HContext;
};


class HContext {
public:
    HContext() : hsCurrent(0) {
    }

    void setInitialHsState(HState* hState) {
        hsCurrent = hState;
//		hState->write();
    }
    
    ERROR write() {
		return hsCurrent->write();
	}

    ERROR on_start_document();
    ERROR on_end_document();
    ERROR on_start_element(const Glib::ustring& name,
                           const xmlpp::SaxParser::AttributeList& attributes);
    ERROR on_end_element(const Glib::ustring& name);
    ERROR on_characters(const Glib::ustring& characters);
    ERROR on_comment(const Glib::ustring& text);
    ERROR on_warning(const Glib::ustring& text);
    ERROR on_error(const Glib::ustring& text);
    ERROR on_fatal_error(const Glib::ustring& text);
    ERROR on_cdata_block (const Glib::ustring& text);

private:
    HState* hsCurrent;
};


class HParser : public xmlpp::SaxParser {
public:
    HParser() : xmlpp::SaxParser(), cSocket(new csocket()), hContext(new HContext()) {
		std::locale::global(std::locale(""));
		set_substitute_entities(true);
	}
	
    ~HParser() override {
        delete hContext;
		delete cSocket;
    }
    
	ERROR connect() {
		ERROR error = ERROR::E_SUCCESS;
		
		if (cSocket->connect(Config::getEntry("HarmonyHub").c_str(), HARMONYPORT) == SUCCESS) {
			error = hContext->write();
		} else {
			error = ERROR::E_NOT_CONNECTED;
		}
		return error;	
	}
	
    HContext* getHContext() {
        return hContext;
    }
    
    HWriter* getWriter(std::string text = "") {
		return new HWriter(cSocket, text);
	}
	
	XmlStanza getXmlStanza() {
		return XmlStanza(cSocket);
	}

    ERROR hParse();

protected:
    //overrides:
    void on_start_document() override;
    void on_end_document() override;
    void on_start_element(const Glib::ustring& name,
                          const AttributeList& attributes) override;
    void on_end_element(const Glib::ustring& name) override;
    void on_characters(const Glib::ustring& characters) override;
    void on_comment(const Glib::ustring& text) override;
    void on_warning(const Glib::ustring& text) override;
    void on_error(const Glib::ustring& text) override;
    void on_fatal_error(const Glib::ustring& text) override;
    void on_cdata_block (const Glib::ustring& text) override;

private:
    void p_on_start_document();
	void p_on_end_document();
    void p_on_start_element(const Glib::ustring& name,
                            const AttributeList& attributes);
    void p_on_end_element(const Glib::ustring& name);
    void p_on_characters(const Glib::ustring& characters);
    void p_on_comment(const Glib::ustring& text);
    void p_on_warning(const Glib::ustring& text);
    void p_on_error(const Glib::ustring& text);
    void p_on_fatal_error(const Glib::ustring& text);
    void p_on_cdata_block (const Glib::ustring& text);

private:
	csocket* cSocket;
    HContext* hContext;
    ERROR error;
};


class HStateAdapter : public HState {
public:
    HStateAdapter(std::string hsName, HWriter* writer = 0) : HState(hsName, writer) {
        // in concrete states set the name of the state
    }

    ~HStateAdapter() {}
    
protected:
    // state change notifications
    virtual ERROR on_reactivate(std::string deactivatedChild) override;
    virtual ERROR on_activate_error(std::string next) override;

    // xml-events
    virtual ERROR on_start_document() override;
    virtual ERROR on_end_document() override;
    virtual ERROR on_start_element(const Glib::ustring& name,
                                   const xmlpp::SaxParser::AttributeList& attributes) override;
    virtual ERROR on_end_element(const Glib::ustring& name) override;
    virtual ERROR on_characters(const Glib::ustring& characters) override;
    virtual ERROR on_comment(const Glib::ustring& text) override;
    virtual ERROR on_warning(const Glib::ustring& text) override;
    virtual ERROR on_error(const Glib::ustring& text) override;
    virtual ERROR on_fatal_error(const Glib::ustring& text) override;
    virtual ERROR on_cdata_block (const Glib::ustring& text) override;
};

} // END namespace LOGI

#endif
