#define CURL_STATICLIB
#include <curl/curl.h>

int main(int argc, char** argv) {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		return 1;
	}

	curl_global_cleanup();
	return 0;
}
