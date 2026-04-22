#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <../include/shader.h>
#include <../include/camera.h>

#include <iostream>

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float mixValue = 0.2f;

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
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// if (!gladLoadGL()){ // I guess it's a different way to check on the initialisation of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	// build and compile the shader program
	Shader myShader("shaders/shader.vs", "shaders/shader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
    	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 0.0f,  1.0f,  0.0f),
		glm::vec3( 0.0f,  2.0f,  0.0f),

		glm::vec3( 0.0f,  0.0f, -1.0f),
		glm::vec3( 0.0f,  1.0f, -1.0f),
		glm::vec3( 0.0f,  2.0f, -1.0f),

		glm::vec3( 0.0f,  0.0f, -2.0f),
		glm::vec3( 0.0f,  1.0f, -2.0f),
		glm::vec3( 0.0f,  2.0f, -2.0f),


		glm::vec3( 1.0f,  0.0f,  0.0f),
		glm::vec3( 1.0f,  1.0f,  0.0f),
		glm::vec3( 1.0f,  2.0f,  0.0f),

		glm::vec3( 1.0f,  0.0f, -1.0f),
		glm::vec3( 1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f,  2.0f, -1.0f),

		glm::vec3( 1.0f,  0.0f, -2.0f),
		glm::vec3( 1.0f,  1.0f, -2.0f),
		glm::vec3( 1.0f,  2.0f, -2.0f),


		glm::vec3( 2.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  1.0f,  0.0f),
		glm::vec3( 2.0f,  2.0f,  0.0f),

		glm::vec3( 2.0f,  0.0f, -1.0f),
		glm::vec3( 2.0f,  1.0f, -1.0f),
		glm::vec3( 2.0f,  2.0f, -1.0f),

		glm::vec3( 2.0f,  0.0f, -2.0f),
		glm::vec3( 2.0f,  1.0f, -2.0f),
		glm::vec3( 2.0f,  2.0f, -2.0f),


		glm::vec3( 3.0f,  0.0f,  0.0f),
		glm::vec3( 3.0f,  1.0f,  0.0f),
		glm::vec3( 3.0f,  2.0f,  0.0f),

		glm::vec3( 3.0f,  0.0f, -1.0f),
		glm::vec3( 3.0f,  1.0f, -1.0f),
		glm::vec3( 3.0f,  2.0f, -1.0f),

		glm::vec3( 3.0f,  0.0f, -2.0f),
		glm::vec3( 3.0f,  1.0f, -2.0f),
		glm::vec3( 3.0f,  2.0f, -2.0f),


		glm::vec3( 4.0f,  0.0f,  0.0f),
		glm::vec3( 4.0f,  1.0f,  0.0f),
		glm::vec3( 4.0f,  2.0f,  0.0f),

		glm::vec3( 4.0f,  0.0f, -1.0f),
		glm::vec3( 4.0f,  1.0f, -1.0f),
		glm::vec3( 4.0f,  2.0f, -1.0f),

		glm::vec3( 4.0f,  0.0f, -2.0f),
		glm::vec3( 4.0f,  1.0f, -2.0f),
		glm::vec3( 4.0f,  2.0f, -2.0f),


		glm::vec3( 5.0f,  0.0f,  0.0f),
		glm::vec3( 5.0f,  1.0f,  0.0f),
		glm::vec3( 5.0f,  2.0f,  0.0f),

		glm::vec3( 5.0f,  0.0f, -1.0f),
		glm::vec3( 5.0f,  1.0f, -1.0f),
		glm::vec3( 5.0f,  2.0f, -1.0f),

		glm::vec3( 5.0f,  0.0f, -2.0f),
		glm::vec3( 5.0f,  1.0f, -2.0f),
		glm::vec3( 5.0f,  2.0f, -2.0f),

		glm::vec3( 3.0f,  0.0f,  1.0f),
		glm::vec3( 3.0f,  1.0f,  1.0f),
		glm::vec3( 3.0f,  2.0f,  1.0f),

		glm::vec3( 3.0f,  0.0f,  2.0f),
		glm::vec3( 3.0f,  1.0f,  2.0f),
		glm::vec3( 3.0f,  2.0f,  2.0f),

		glm::vec3( 3.0f,  0.0f,  3.0f),
		glm::vec3( 3.0f,  1.0f,  3.0f),
		glm::vec3( 3.0f,  2.0f,  3.0f),


		glm::vec3( 4.0f,  0.0f,  1.0f),
		glm::vec3( 4.0f,  1.0f,  1.0f),
		glm::vec3( 4.0f,  2.0f,  1.0f),

		glm::vec3( 4.0f,  0.0f,  2.0f),
		glm::vec3( 4.0f,  1.0f,  2.0f),
		glm::vec3( 4.0f,  2.0f,  2.0f),

		glm::vec3( 4.0f,  0.0f,  3.0f),
		glm::vec3( 4.0f,  1.0f,  3.0f),
		glm::vec3( 4.0f,  2.0f,  3.0f),


		glm::vec3( 5.0f,  0.0f,  1.0f),
		glm::vec3( 5.0f,  1.0f,  1.0f),
		glm::vec3( 5.0f,  2.0f,  1.0f),

		glm::vec3( 5.0f,  0.0f,  2.0f),
		glm::vec3( 5.0f,  1.0f,  2.0f),
		glm::vec3( 5.0f,  2.0f,  2.0f),

		glm::vec3( 5.0f,  0.0f,  3.0f),
		glm::vec3( 5.0f,  1.0f,  3.0f),
		glm::vec3( 5.0f,  2.0f,  3.0f),


		glm::vec3( 4.0f,  3.0f,  3.0f),
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// --- load and create a texture ---
	unsigned int texture1, texture2;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1); // any future GL_TEXTURE_2D operations will have effect on this texture object

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image, create texture and  generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("assets/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture2
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2); // any future GL_TEXTURE_2D operations will have effect on this texture object

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image, create texture and  generate mipmaps
	data = stbi_load("assets/awesomeface.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture2" << std::endl;
	}
	stbi_image_free(data);

	// specify for each sampler which texture unit it belongs to
	myShader.use(); // must active the shader before doing any work at all
	myShader.setInt("texture1", 0);
	myShader.setInt("texture2", 1);

	
	while (!glfwWindowShouldClose(window)){
		// frame logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// inputs
		processInput(window);
		
		// rendering related
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		
		// activate shader
		myShader.use();
		myShader.setFloat("mixValue", mixValue);
		
		// camera/view transformations
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		myShader.setMat4("projection", projection);

		glm::mat4 view = camera.GetViewMatrix();
		myShader.setMat4("view", view);

		// render container
		glBindVertexArray(VAO);
		for(unsigned int i = 0; i < 82; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			
			myShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		mixValue += 0.01f;
		if(mixValue >= 1.0f)
			mixValue = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		mixValue -= 0.01f;
		if(mixValue <= 0.0f)
			mixValue = 0.0f;
	}
}

// whenever the window size changes (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if(firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos -lastX;
	float yoffset = lastY - ypos;
	
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}