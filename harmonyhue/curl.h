#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>

#include <iostream>
#include <list>
#include <string>
#include <string.h>


std::string post(std::string url, std::string content, std::list<std::string>& header);

class EasyCurl {
private:
    EasyCurl() {}

    ~EasyCurl() {}

public:
    static EasyCurl* instance() {
        if(!easyCurl) {
            easyCurl = new EasyCurl();
			curl_global_init(CURL_GLOBAL_ALL);
        }
        return easyCurl;
    }

    static void cleanUp() {
        if (easyCurl) {
			curl_global_cleanup();
            delete easyCurl;
			easyCurl = 0;
        }
    }

    int post(std::string url, std::string content, std::list<std::string>& header, std::string& response, bool async = false);
    int put(std::string url, std::string content, std::string& response, bool async = false);

private:
    static size_t read_cb(void *ptr, size_t size, size_t nmemb, void *stream);
    static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *stream);
	
    static EasyCurl* easyCurl;
};

#endif

