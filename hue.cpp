
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * HTTP PUT with easy interface and read callback
 * </DESC>
 */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>

/*
 * This example shows a HTTP PUT operation. PUTs a file given as a command
 * line argument to the URL also given on the command line.
 *
 * This example also uses its own read callback.
 *
 * Here's an article on how to setup a PUT handler for Apache:
 * http://www.apacheweek.com/features/put
 */

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, (FILE*) stream);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);

  return retcode;
}


static size_t read_callback_on(void *ptr, size_t size, size_t nmemb, void *stream) {
	
	strcpy((char*) ptr, "{\"flag\" : true}");
	
	return strlen("{\"flag\" : true}");
}


static size_t read_callback_off(void *ptr, size_t size, size_t nmemb, void *stream) {
	
	strcpy((char*) ptr, "{\"flag\" : false}");
	
	return strlen("{\"flag\" : false}");
}

static size_t function_pt(void *ptr, size_t size, size_t nmemb, void *stream){
    printf("%s\n", ptr);
	
	return size*nmemb;
}

int main3(int status)
{
  CURL *curl;
  CURLcode res;

  const char *url = "http://philips-hue.feuerweg.vchrist.at/api/qjayddogNNF6X4C9IiVTjvd-XclsCXhT8sF1MyvL/sensors/7/state";

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
	/* we want to use our own read function */
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, status == 0 ? read_callback_off : read_callback_on);
	
	/* we want to see the response */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, function_pt);
	
    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* HTTP PUT please */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    /* specify target URL, and note that this URL should include a file
       name, not only a directory */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, 0);

    /* provide the size of the upload, we specicially typecast the value
       to curl_off_t since we must be sure to use the correct data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t) strlen(status == 0 ? "{\"flag\" : false}" : "{\"flag\" : true}"));

    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return 0;
}
