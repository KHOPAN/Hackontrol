#include <stdio.h>
#include <khopanwin32.h>

#define TEXT_HELLO_WORLD "Hello, world!"

static const char* globalText = TEXT_HELLO_WORLD;

int main(int argc, char** argv) {
	const char* first = TEXT_HELLO_WORLD;
	unsigned long long firstAddress = (unsigned long long) &first;
	unsigned long long secondAddress = (unsigned long long) &globalText;
	printf("First: %llu\nSecond: %llu\nSame: %s\n", firstAddress, secondAddress, firstAddress == secondAddress ? "True" : "False");
	return 0;
}
