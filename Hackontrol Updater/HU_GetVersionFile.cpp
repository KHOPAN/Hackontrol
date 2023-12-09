#include <string>
#include <Windows.h>
#include "HackontrolUpdater.h"

std::string buffer;
static size_t write_data(char*, size_t, size_t, void*);

const char* HU_GetVersionFile(CURL* curl) {
	CURLcode code = curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/version.json");
	
	if(code != CURLE_OK) {
		ExitProcess(code);
		return NULL;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

	if(code != CURLE_OK) {
		ExitProcess(code);
		return NULL;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

	if(code != CURLE_OK) {
		ExitProcess(code);
		return NULL;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		ExitProcess(code);
		return NULL;
	}

	return buffer.c_str();
}

static size_t write_data(char* data, size_t size, size_t count, void* ignored) {
	size_t written = count * size;
	std::string string(data, written);
	buffer += string;
	return written;
}
