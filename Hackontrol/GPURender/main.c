#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "main.h"
#include "shader.h"
#include "resource.h"
#include "title.h"

static SHADERDATASTRUCT ShaderList[] = {
	{IDR_RCDATA1, GL_VERTEX_SHADER},
	{IDR_RCDATA2, GL_FRAGMENT_SHADER}
};

int InitializeGPURender() {
	int error = 1;

	if(!glfwInit()) {
		MessageBoxW(NULL, L"Initialization failed", L"GLFW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return error;
	}

	int width = 600;
	int height = 400;
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Window", NULL, NULL);

	if(!window) {
		MessageBoxW(NULL, L"Window creation failed", L"GLFW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto terminate;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	if(!monitor) {
		MessageBoxW(NULL, L"Primary monitor could not be found", L"GLFW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto terminate;
	}

	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
	int screenWidth = videoMode->width;
	int screenHeight = videoMode->height;
	int windowX = (screenWidth - width) / 2;
	int windowY = (screenHeight - height) / 2;
	glfwSetWindowPos(window, windowX, windowY);
	glfwMakeContextCurrent(window);

	if(glewInit()) {
		MessageBoxW(NULL, L"Initialization failed", L"GLEW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto terminate;
	}

	char* title = calculateWindowTitle();
	glfwSetWindowTitle(window, title);
	free(title);
	unsigned int positionBuffer;
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	float positions[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		-0.5f, 0.5f,
		0.5f, 0.5f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	unsigned int indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	unsigned int indices[] = {
		2, 0, 1,
		1, 3, 2
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	int shaderProgram = LoadShaderProgram(ShaderList, sizeof(ShaderList) / sizeof(ShaderList[0]));

	if(!shaderProgram) {
		goto terminate;
	}

	glUseProgram(shaderProgram);

	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);
	error = 0;
terminate:
	glfwTerminate();
	return error;
}
