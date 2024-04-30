#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "main.h"

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

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	float positions[] = {
		-0.5f, -0.5f,
		0.0f, 0.5f,
		0.5f, -0.5f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	error = 0;
terminate:
	glfwTerminate();
	return error;
}
