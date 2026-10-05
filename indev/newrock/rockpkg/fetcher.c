#include <string.h>
#include <curl/curl.h>

struct Response 
{
	char *data;
	unsigned long size;
	unsigned long cap;
};

static unsigned long write_callback
(char *data, unsigned long size, unsigned long nmemb, void *userdata)
{
	struct Response *response = userdata;
	unsigned long bytes = size * nmemb;

	if (response->size + bytes >= response->cap) return 0; /* not enough space */

	memcpy(response->data + response->size, data, bytes);
	response->size += bytes;

	response->data[response->size] = '\0';

	return bytes;
}

unsigned long fetch
(char *buff, unsigned long buffsize, char *url)
{
	CURL *curl = curl_easy_init();
	if (!curl) return 0; 

	struct Response response = {
		.data = buff,
		.size = 0,
		.cap = buffsize,
	};


	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode result = curl_easy_perform(curl);

	if (result != CURLE_OK) buff[0] = '\0';

	curl_easy_cleanup(curl);

	return response.size;
}
