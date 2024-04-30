#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "title.h"

#define TITLE "OpenGL Window"
#define GLSL "GLSL"

char* calculateWindowTitle() {
	const char* versionOpenGL = glGetString(GL_VERSION);
	const char* versionGLSL = glGetString(GL_SHADING_LANGUAGE_VERSION);
	size_t titleLength = strlen(TITLE);
	size_t versionOpenGLLength = strlen(versionOpenGL);
	size_t glslLength = strlen(GLSL);
	size_t versionGLSLLength = strlen(versionGLSL);
	size_t bufferLength = titleLength + 1 + versionOpenGLLength + 1 + glslLength + 1 + versionGLSLLength + 1;
	char* buffer = malloc(bufferLength * sizeof(char));
	
	if(!buffer) {
		return TITLE;
	}

	for(size_t i = 0; i < titleLength; i++) {
		buffer[i] = TITLE[i];
	}

	buffer[titleLength] = ' ';

	for(size_t i = 0; i < versionOpenGLLength; i++) {
		buffer[i + titleLength + 1] = versionOpenGL[i];
	}

	buffer[titleLength + versionOpenGLLength + 1] = ' ';

	for(size_t i = 0; i < glslLength; i++) {
		buffer[i + titleLength + versionOpenGLLength + 2] = GLSL[i];
	}

	buffer[glslLength + titleLength + versionOpenGLLength + 2] = ' ';

	for(size_t i = 0; i < versionGLSLLength; i++) {
		buffer[i + glslLength + titleLength + versionOpenGLLength + 3] = versionGLSL[i];
	}

	buffer[bufferLength - 1] = 0;
	return buffer;
}
