#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <../include/shaders/shader.h>

//#include <glm/glm.hpp>

#include <iostream>

void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// whenever the window size changes (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//const char *vertexShaderSource = "#version 330 core\n"
//    "layout (location = 0) in vec3 aPos;\n"
//	"layout (location = 1) in vec3 aColor;\n"
//	//"out vec4 vertexColor;\n"
//	"out vec3 ourColor;\n"
//    "void main()\n"
//    "{\n"
//    "   gl_Position = vec4(aPos, 1.0);\n"
//	"   ourColor = aColor;\n"
//	//"	vertexColor = vec4(0.5, 0.0, 0.0, 1.0);\n"
//    "}\0";
//const char *fragmentShaderSource = "#version 330 core\n"
//    "out vec4 FragColor;\n"
//	//"in vec4 vertexColor;\n"
//	//"uniform vec4 ourColor;\n"
//	"in vec3 ourColor;\n"
//    "void main()\n"
//    "{\n"
//    //"   FragColor = vertexColor;\n"
//	"   FragColor = vec4(ourColor, 1.0);\n"
//    "}\n\0";

int main(void){
	if (!glfwInit())
	{
		std::cerr << "Failed to init GLFW\n";
		return -1;
	} 

	// for now I'll be using version 3.3 (LearnOpenGL tutorials' version)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// if (!gladLoadGL()){ // I guess it's a different way to check on the initialisation of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	// build and compile our shader program
	Shader myShader("shaders/shader.vs", "shaders/shader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
	   -0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	// bottom left
	    0.0f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f	// top left
	};
	//unsigned int indices[] = {
	//	0, 1, 3,	// first triangle
	//	1, 2, 3		// second triangle
	//};

	//unsigned  int VBO, VAO, EBO;
	unsigned  int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	// some information to startup 'cause I like it
	std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";

	glViewport(0, 0, 800, 600);

	while (!glfwWindowShouldClose(window)){
		// inputs
		processInput(window);
		
		// rendering related
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// activate the shader first (if not done first, updates will not matter when using uniforms)
		//glUseProgram(shaderProgram);

		// draws
		//float timeValue = glfwGetTime();
		//float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		//int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		//glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
		
		myShader.use();
		myShader.setFloat("offset", 0.5f);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}