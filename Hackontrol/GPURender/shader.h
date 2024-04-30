#pragma once

typedef struct tagSHADERDATASTRUCT {
	unsigned int shaderResource;
	unsigned int shaderType;
} SHADERDATASTRUCT;

int LoadShaderProgram(const SHADERDATASTRUCT* shaders, unsigned long shaderCount);
