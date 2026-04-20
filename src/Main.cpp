#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main(void){
	if (!glfwInit()){
		std::cerr << "Failed to init GLFW\n";
		return -1;
	} 

	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello", NULL, NULL);
	if (!window){
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL()){
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";


	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}