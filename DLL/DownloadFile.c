#include "definition.h"

int indexOf(const char*, char);

EXPORT DownloadFile(HWND window, HINSTANCE instance, LPSTR argument, int command) {

}

int indexOf(const char* text, char character) {
	int length = strlen(text);

	for(int i = 0; i < length; i++) {
		if(text[i] == character) {
			return i;
		}
	}

	return -1;
}
