#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>

#include <iostream>
#include <list>
#include <string>
#include <string.h>


static size_t read_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
	static int count = 0;
    std::string* string = (std::string*) stream;

    size_t bufferSpace = nmemb * size;
    size_t strLength = string->length();
    size_t sendLength = 0;

    if (strLength - count > bufferSpace) {
    	sendLength = bufferSpace;
    } else {
    	sendLength = strLength - count;
    }

    memcpy(ptr, string->substr(count).c_str(), sendLength);

    std::cout << "::::: ->" << string->substr(count).c_str() << std::endl;

    count += sendLength;

    if(count == strLength) {
    	count = 0;
    }

    return sendLength;
}

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
	std::string* string = (std::string*) stream;

	*((char*) ptr + size * nmemb) = 0;

	*(std::string*) stream = std::string((char*) ptr);

	return size*nmemb;
}



int main3(int status);

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

    int post(std::string url, std::string content, std::list<std::string>& header, std::string& response) {
        CURL *curl = curl_easy_init();
        CURLcode res;

        if(curl) {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) content.length());


            struct curl_slist *chunk = NULL;
            std::list<std::string>::iterator it;
            for (it = header.begin(); it != header.end(); it++) {
                chunk = curl_slist_append(chunk, (*it).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));

            curl_easy_cleanup(curl);
        }

        return 0;
    }

    std::string put(std::string url, std::string content, std::list<std::string>& header) {
        CURL* curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

            curl_easy_setopt(curl, CURLOPT_READDATA, content.c_str());
            curl_easy_setopt(curl, CURLOPT_PUT, 1L);

            struct curl_slist *chunk = NULL;
            std::list<std::string>::iterator it;
            for (it = header.begin(); it != header.end(); it++) {
                chunk = curl_slist_append(chunk, (*it).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) strlen("{\"email\":\"volker.christian@liwest.at\",\"password\":\"pentium5\"}"));

            int res = curl_easy_perform(curl);
        }

        curl_easy_cleanup(curl);
    }

    std::string get(std::string url, std::list<std::string>& header) {
    }

private:
    static EasyCurl* easyCurl;
};


#endif

