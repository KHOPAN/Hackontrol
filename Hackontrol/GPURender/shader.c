#include <GL/glew.h>
#include <khopanerror.h>
#include "shader.h"
#include "main.h"

static unsigned int compileShader(HINSTANCE instance, const SHADERDATASTRUCT shaderData) {
	unsigned int shaderResource = shaderData.shaderResource;
	unsigned int shaderType = shaderData.shaderType;
	HRSRC resourceHandle = FindResourceW(instance, MAKEINTRESOURCE(shaderResource), RT_RCDATA);

	if(!resourceHandle) {
		KHWin32DialogErrorW(GetLastError(), L"FindResourceW");
		return 0;
	}

	DWORD resourceSize = SizeofResource(instance, resourceHandle);

	if(!resourceSize) {
		KHWin32DialogErrorW(GetLastError(), L"SizeofResource");
		return 0;
	}

	HGLOBAL resource = LoadResource(instance, resourceHandle);

	if(!resource) {
		KHWin32DialogErrorW(GetLastError(), L"LoadResource");
		return 0;
	}

	char* data = LockResource(resource);

	if(!data) {
		KHWin32DialogErrorW(GetLastError(), L"LockResource");
		return 0;
	}

	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &data, &resourceSize);
	glCompileShader(shader);
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(!status) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* buffer = malloc(length * sizeof(char));

		if(!buffer) {
			KHWin32DialogErrorW(ERROR_OUTOFMEMORY, L"malloc");
			return 0;
		}

		glGetShaderInfoLog(shader, length, &length, buffer);
		MessageBoxA(NULL, buffer, "OpenGL Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		free(buffer);
	}

	return shader;
}

int LoadShaderProgram(const SHADERDATASTRUCT* shaders, unsigned long shaderCount) {
	if(!shaders) {
		MessageBoxW(NULL, L"Invalid shader", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return 0;
	}

	if(shaderCount < 1) {
		MessageBoxW(NULL, L"Invalid shader count", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return 0;
	}

	unsigned int program = glCreateProgram();
	unsigned int* shaderIdentifiers = malloc(shaderCount * sizeof(unsigned int));

	if(!shaderIdentifiers) {
		KHWin32DialogErrorW(ERROR_OUTOFMEMORY, L"malloc");
		return 0;
	}

	HINSTANCE instance = GetProgramInstance();

	for(unsigned long i = 0; i < shaderCount; i++) {
		const SHADERDATASTRUCT shaderData = shaders[i];
		unsigned int shader = compileShader(instance, shaderData);

		if(!shader) {
			return 0;
		}

		glAttachShader(program, shader);
		shaderIdentifiers[i] = shader;
	}

	glLinkProgram(program);
	glValidateProgram(program);

	for(unsigned long i = 0; i < shaderCount; i++) {
		unsigned int shader = shaderIdentifiers[i];
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}

	free(shaderIdentifiers);
	return program;
}
