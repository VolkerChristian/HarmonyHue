#include "harmony.h"
#include <glibmm/convert.h> //For Glib::ConvertError

#include <iostream>

void HarmonyParser::on_start_document() {
	state = adapter->on_start_document();
    std::cout << "on_start_document()" << std::endl;
}

void HarmonyParser::on_end_document() {
	state = adapter->on_end_document();
    std::cout << "on_end_document()" << std::endl;
}

void HarmonyParser::on_start_element(const Glib::ustring& name,
                                   const AttributeList& attributes) {
	state = adapter->on_start_element(name, attributes);
	
    std::cout << "node name=" << name << std::endl;

    // Print attributes:
    for(const auto& attr_pair : attributes)
    {
        try
        {
            std::cout << "    " << attr_pair.name;
        }
        catch(const Glib::ConvertError& ex)
        {
            std::cerr << "MySaxParser::on_start_element(): Exception caught while converting name for std::cout: " << ex.what() << std::endl;
        }

        try
        {
            std::cout << " = " <<  attr_pair.value << std::endl;
        }
        catch(const Glib::ConvertError& ex)
        {
            std::cerr << "MySaxParser::on_start_element(): Exception caught while converting value for std::cout: " << ex.what() << std::endl;
        }
    }
}

void HarmonyParser::on_end_element(const Glib::ustring&  name) {
	state = adapter->on_end_element(name);
	
    std::cout << "on_end_element()" << name << std::endl;
}

void HarmonyParser::on_characters(const Glib::ustring& text) {
	state = adapter->on_characters(text);
	
    try
    {
        std::cout << "on_characters(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_characters(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

void HarmonyParser::on_comment(const Glib::ustring& text) {
	state = adapter->on_comment(text);
	
    try
    {
        std::cout << "on_comment(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_comment(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

void HarmonyParser::on_warning(const Glib::ustring& text) {
	state = adapter->on_warning(text);
	
    try
    {
        std::cout << "on_warning(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_warning(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

void HarmonyParser::on_error(const Glib::ustring& text) {
	state = adapter->on_error(text);
	
    try
    {
        std::cout << "on_error(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_error(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

void HarmonyParser::on_fatal_error(const Glib::ustring& text) {
	state = adapter->on_fatal_error(text);
	
    try
    {
        std::cout << "on_fatal_error(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_characters(): Exception caught while converting value for std::cout: " << ex.what() << std::endl;
    }
}


void HarmonyParser::on_cdata_block(const Glib::ustring& text) {
	state = adapter->on_cdata_block(text);
	
    try
    {
        std::cout << "on_cdata_block(): " << text << std::endl;
    }
    catch(const Glib::ConvertError& ex)
    {
        std::cerr << "MySaxParser::on_characters(): Exception caught while converting text for std::cout: " << ex.what() << std::endl;
    }
}

HarmonyParser::State HarmonyParser::startParse(HarmonyParserAdapter* adapter) {
	this->adapter = adapter;
	
	std::string text = adapter->getPreParseText();
	
	std::cout << "Write: " << text << std::endl;
	commandSocket->write(text.c_str(), text.length());
	
    // Incremental parsing, sometimes useful for network connections:
    HarmonyParser::State return_code = HarmonyParser::State::next;
	
    try {
        char buffer[64];
        const size_t buffer_size = sizeof(buffer) / sizeof(char);
        int bytesRead = 0;
		bool canRead = false;
		float remainingSecs = 30;
		bool heardBeat = false;
		
        do {
			heardBeat = false;
			if (commandSocket->canRead(&canRead, &remainingSecs, remainingSecs) == FAILURE) {
				return_code == HarmonyParser::State::error;
				break;
			}
            if (canRead) {
                std::memset(buffer, 0, buffer_size);
                bytesRead = commandSocket->read(buffer, buffer_size-1, false);
                if(bytesRead > 0) {
                    // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
                    // instead of Glib::ustring::ustring( const char*, size_type ) because it
                    // expects the length of the string in characters, not in bytes.
                    Glib::ustring input(buffer, buffer + bytesRead);
                    parse_chunk(input);
                }
            } else {
				std::cout << "Heardbeat" << std::endl;
                commandSocket->write("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>", strlen("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>"));
				remainingSecs = 30;
				heardBeat = true;
            }
        }
        while(((bytesRead > 0 || errno == EINTR || !canRead) && state == State::cont) || heardBeat);
    }
    catch(const xmlpp::exception& ex) {
        std::cerr << "Incremental parsing, libxml++ exception: " << ex.what() << std::endl;
        return_code = HarmonyParser::State::error;
    }

    return return_code;
}
