#include <Windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR argument, int command) {
	curl_global_init(CURL_GLOBAL_ALL);
	CURL* curl = curl_easy_init();
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0;
}
