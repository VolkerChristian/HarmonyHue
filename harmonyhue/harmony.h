#ifndef __LIBXMLPP_EXAMPLES_MYPARSER_H
#define __LIBXMLPP_EXAMPLES_MYPARSER_H

#include <iostream>

#include <jansson.h>
#include <libxml++/libxml++.h>

#include "base64.h"
#include "csocket.h"

class HarmonyParserAdapter;

class HarmonyParser : public xmlpp::SaxParser
{
public:
    enum State {
        cont,
        next,
        error
    };

public:
    HarmonyParser(csocket* commandSocket) : xmlpp::SaxParser() {
        std::locale::global(std::locale(""));
        setCommandSocket(commandSocket);
        set_substitute_entities(true);
    }

    void setCommandSocket(csocket* commandSocket) {
        this->commandSocket = commandSocket;
    }

    ~HarmonyParser() override {};

    HarmonyParser::State startParse(HarmonyParserAdapter* adapter);

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
    csocket* commandSocket = 0;
    bool state = true;
    HarmonyParserAdapter* adapter;
};


class HarmonyParserAdapter {
public:
    virtual HarmonyParser::State on_start_document() {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_end_document() {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_start_element(const Glib::ustring& name,
            const xmlpp::SaxParser::AttributeList& attributes) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_end_element(const Glib::ustring& name) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_characters(const Glib::ustring& characters) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_comment(const Glib::ustring& text) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_warning(const Glib::ustring& text) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_error(const Glib::ustring& text) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_fatal_error(const Glib::ustring& text) {
        return HarmonyParser::State::cont;
    }
    virtual HarmonyParser::State on_cdata_block (const Glib::ustring& text) {
        return HarmonyParser::State::cont;
    }
    virtual std::string getPreParseText() {
        return std::string();
    }
};

class AuthorizeParser : public HarmonyParserAdapter {
public:
    AuthorizeParser(std::string& strAuthorizationToken) : strAuthorizationToken(strAuthorizationToken) {}
    std::string getPreParseText() {
    }

private:
    std::string& strAuthorizationToken;
};


class ConnectAdapter : public HarmonyParserAdapter {
public:
    HarmonyParser::State on_start_element(const Glib::ustring& name,
                                          const xmlpp::SaxParser::AttributeList& attributes) {
        HarmonyParser::State state = HarmonyParser::State::cont;

        if (name == "stream:stream") {
            int counter = 0;
            for(const auto& attr_pair : attributes)	{
                counter = counter + (attr_pair.name == "to") ? 1 : 0;
                counter = counter + (attr_pair.name == "xmlns:stream") ? 1 : 0;
                counter = counter + (attr_pair.name == "xmlns") ? 1 : 0;
                counter = counter + (attr_pair.name == "xml:lang") ? 1 : 0;
                counter = counter + (attr_pair.name == "version") ? 1 : 0;
                std::cout << "  Attribute name=" <<  attr_pair.name << ", value=" << attr_pair.value << std::endl;
            }
        } else if (name == "stream:features") {
        } else if (name == "mechanisms") {
        } else if (name == "mechanism") {
        }

        return state;
    }

    HarmonyParser::State on_end_element(const Glib::ustring& name) {
        HarmonyParser::State state = HarmonyParser::State::cont;

        if (name == "mechanism") {
        } else if (name == "mechanisms") {
        } else if (name == "stream:features") {
            state = HarmonyParser::State::next;
        }
        if (name == "stream::stream") {
            state = HarmonyParser::State::next;
        }

        return state;
    }

    std::string getPreParseText() {
        return "<stream:stream to='connect.logitech.com' xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' xml:lang='en' version='1.0'>";
    }
    /* <- Expect: <?xml version='1.0' encoding='iso-8859-1'?><stream:stream from='' id='XXXXXXXX' version='1.0' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'><stream:features><mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><mechanism>PLAIN</mechanism></mechanisms></stream:features> */
};

class LoginAdapter : public HarmonyParserAdapter {
public:
    LoginAdapter(std::string userName, std::string password) : userName(userName), password(password) {
    }

    HarmonyParser::State on_end_element(const Glib::ustring& name) {
        HarmonyParser::State state = HarmonyParser::State::next;

        if (name != "success") {
            state = HarmonyParser::State::error;
        }

        return state;
    }

    std::string getPreParseText() {
        std::string data = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">";
        std::string tmp = "\0";
        tmp.append(userName);
        tmp.append("\0");
        tmp.append(password);
        data.append(base64_encode(tmp.c_str(), tmp.length()));
        data.append("</auth>");
        return data;
    }

private:
    std::string userName;
    std::string password;
};

class AuthorizationTokenAdapter : public HarmonyParserAdapter {
public:
	AuthorizationTokenAdapter(std::string& authorizationToken) : authorizationToken(authorizationToken) {
	}
	
    HarmonyParser::State on_cdata_block(const Glib::ustring& text) {
        HarmonyParser::State state = HarmonyParser::State::cont;

        int pos = (int) text.find("identity=");
        if(pos != std::string::npos) {

            std::string strAuthorizationToken = text.substr(pos + std::string("identity=").length());

            pos = (int) strAuthorizationToken.find(":status=succeeded");
            if(pos != std::string::npos) {
                authorizationToken = strAuthorizationToken.substr(0, pos);
				std::cout << "Token: " << authorizationToken << std::endl;
            } else {
                state = HarmonyParser::State::error;
            }
        } else {
            state = HarmonyParser::State::error;
        }
        
        return state;
    }
    
    HarmonyParser::State on_start_element(const Glib::ustring & name, const xmlpp::SaxParser::AttributeList & attributes) {
		HarmonyParser::State state = HarmonyParser::State::cont;
		
		if (name != "iq" && name != "oa") {
			state = HarmonyParser::State::error;
		}
		
		return state;
	}
    
    HarmonyParser::State on_end_element(const Glib::ustring& text) {
		HarmonyParser::State state = HarmonyParser::State::cont;
		
		if (text == "iq") {
			counter++;
		}
		if (counter == 2) {
			state = HarmonyParser::State::next;
		}
		
		return state;
	}

    std::string getPreParseText() {
        std::string sendData = "<iq type=\"get\" id=\"";
        sendData.append("12345678-1234-5678-1234-123456789012-1");
        sendData.append("\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.connect/vnd.logitech.pair\">token=");
		sendData.append(authorizationToken.c_str());
        sendData.append("n/LCkMcFkXZVc5UhbjT6+1xdX0/2fhC3Kr17x2SrhDRX+x+9dqcjLQZz/F3vkm7E");
        sendData.append(":name=foo#iOS6.0.1#iPhone</oa></iq>");
        return sendData;
    }
    
private:
	std::string& authorizationToken;
	int counter = 0;
};

#endif //__LIBXMLPP_EXAMPLES_MYPARSER_H
