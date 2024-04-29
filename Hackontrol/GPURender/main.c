#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR arguments, _In_ int commandShow) {
	OutputDebugStringW(L"Hello, world!\n");
	int error = 1;

	if(!glfwInit()) {
		MessageBoxW(NULL, L"Initialization failed", L"GLFW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		return error;
	}

	GLFWwindow* window = glfwCreateWindow(600, 400, "OpenGL Window", NULL, NULL);

	if(!window) {
		MessageBoxW(NULL, L"Window creation failed", L"GLFW Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto terminate;
	}

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
