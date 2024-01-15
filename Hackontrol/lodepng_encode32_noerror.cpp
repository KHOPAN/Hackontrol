#include "lodepng.h"
#include "definition.h"

unsigned int lodepng_encode32_noerror(unsigned char** output, size_t* outputSize, const unsigned char* image, unsigned int width, unsigned int height) {
	return lodepng_encode32(output, outputSize, image, width, height);
}
