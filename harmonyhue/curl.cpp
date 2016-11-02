#include "curl.h"

#include <unistd.h>

#include "hparser.h"

EasyCurl* EasyCurl::easyCurl = 0;

int EasyCurl::post(std::string url, std::string content, std::list<std::string>& header, std::string& response, bool async) {
	if (async) {
		if (fork() != 0) {
			return HARMONY::ERROR::E_SUCCESS;
		}
	}
	
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

    return res;
}

int EasyCurl::put(std::string url, std::string content, std::string& response, bool async) {
	if (async) {
		if (fork() != 0) {
			return HARMONY::ERROR::E_SUCCESS;
		}
	}
	
    CURL* curl = curl_easy_init();
    CURLcode res;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);
        curl_easy_setopt(curl, CURLOPT_READDATA, &content);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        res = curl_easy_perform(curl);
    }

    curl_easy_cleanup(curl);
        
    return res;
}

    
size_t EasyCurl::read_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    static int count = 0;
    size_t sendLength = 0;
    std::string* string = (std::string*) stream;
    size_t strLength = string->length();
    size_t bufferSpace = nmemb * size;
        
    if (count != strLength) {
        if (strLength - count > bufferSpace) {
            sendLength = bufferSpace;
        } else {
            sendLength = strLength - count;
        }

        memcpy(ptr, string->substr(count).c_str(), sendLength);
    
        count += sendLength;
    } else {
        count = 0;
    }
        
    return sendLength;
}

    
size_t EasyCurl::write_cb(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string* string = (std::string*) stream;
    
    *((char*) ptr + size * nmemb) = 0;
    *(std::string*) stream = (char*) ptr;
        
    return size * nmemb;
}
