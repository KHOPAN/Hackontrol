#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <string>
#include <Windows.h>
#include "HackontrolUpdater.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR argument, int command) {
	CURL* curl = HU_InitializeCURL();
	const char* versionFile = HU_GetVersionFile(curl);
	rapidjson::Document document;
	document.Parse(versionFile);
	rapidjson::Value& test = document["hash"];
	MessageBoxA(NULL, test.GetString(), NULL, MB_OK);
	return 0;
}
