#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <cstring> // std::memset()
#include <libxml/parser.h>
#include <errno.h>

#include "myparser.h"
#include "csocket.h"

int main1(csocket* commandcsocket) {
    // Set the global C and C++ locale to the user-configured locale,
    // so we can use std::cout with UTF-8, via Glib::ustring, without exceptions.
    std::locale::global(std::locale(""));

    // Incremental parsing, sometimes useful for network connections:
    auto return_code = EXIT_SUCCESS;
    try {
        char buffer[64];
        const size_t buffer_size = sizeof(buffer) / sizeof(char);
        int bytesRead = 0;
		bool canRead = false;
		float remainingSecs = 30;
		
        std::cout << std::endl << "Incremental SAX Parser:" << std::endl;
		
        //Parse the file:
        MySaxParser parser;
        parser.set_substitute_entities(true);
		
        Glib::ustring document("<begin>");
        parser.parse_chunk(document);
		
        do {
			if (commandcsocket->canRead(&canRead, &remainingSecs, remainingSecs) == FAILURE) {
				break;
			}
            if (canRead) {
                std::memset(buffer, 0, buffer_size);
                bytesRead = commandcsocket->read(buffer, buffer_size-1, false);
                if(bytesRead > 0) {
                    // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
                    // instead of Glib::ustring::ustring( const char*, size_type ) because it
                    // expects the length of the string in characters, not in bytes.
                    Glib::ustring input(buffer, buffer + bytesRead);
                    parser.parse_chunk(input);
                }
            } else {
                commandcsocket->write("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>", strlen("<iq type='get' id='2320426445' from='guest'>\
				<oa xmlns='connect.logitech.com' mime='vnd.logitech.connect/vnd.logitech.ping'>\
				</oa>\
				</iq>"));
				remainingSecs = 30;
            }
        }
        while(bytesRead > 0 || errno == EINTR || !canRead);
		
		document = "</begin>";
        parser.parse_chunk(document);
 
		parser.finish_chunk_parsing();
    }
    catch(const xmlpp::exception& ex) {
        std::cerr << "Incremental parsing, libxml++ exception: " << ex.what() << std::endl;
        return_code = EXIT_FAILURE;
    }

    return return_code;
}
