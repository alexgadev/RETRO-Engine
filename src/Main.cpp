#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/camera.h"
#include "render/shader.h"
#include "render/mesh.h"
#include "render/text_renderer.h"
#include "render/hud.h"

#include <iostream>
#include <sstream>
#include <string>

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadTexture(const char *path);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// HUD
HUD hudController(SCR_WIDTH, SCR_HEIGHT,
					"resources/fonts/Antonio-Bold.ttf", 
					"shaders/textShader.vs", "shaders/textShader.fs",
					"shaders/hudShader.vs", "shaders/hudShader.fs");

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// build and compile the shader program
	Shader lightingShader("shaders/shader.vs", "shaders/color_shader.fs");
	Shader lightCubeShader("shaders/light_cube.vs", "shaders/lightcube_shader.fs");

	TextRenderer textRenderer(SCR_WIDTH, SCR_HEIGHT,
							"resources/fonts/Antonio-Bold.ttf", 
							"shaders/textShader.vs", "shaders/textShader.fs");

	Mesh cube = Mesh::createCube(); // only one needed, model transformations will enable multiple cubes to be created with the same instance

	unsigned int diffuseMap = loadTexture("resources/assets/container2.png");
	unsigned int specularMap = loadTexture("resources/assets/container2_specular.png");
	
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	// ---- fixed-timestep config ----
	const double TICK_RATE = 60.0;              // simulation ticks per second
	const double FIXED_DT  = 1.0 / TICK_RATE;   // seconds advanced by one tick
	double accumulator = 0.0;                   // unsimulated real time carried between frames
	double simTime     = 0.0;                   // deterministic sim clock (advanced only by ticks)

	// ---- per-frame metrics (milliseconds), measured every frame ----
	double frameMs = 0.0, updateMs = 0.0, renderMs = 0.0;

	// ---- HUD readout: averaged over a fixed window so the numbers hold still
	//      long enough to read (raw per-frame values change too fast) ----
	const double HUD_REFRESH = 0.25;   // seconds between on-screen text updates
	double hudTimer   = 0.0;          // real time elapsed since the last refresh
	double accFrameMs = 0.0, accUpdateMs = 0.0, accRenderMs = 0.0, accTickMs = 0.0;
	int    frameCount = 0, tickCount = 0;   // samples gathered during this window
	double dispFrameMs = 0.0, dispUpdateMs = 0.0, dispRenderMs = 0.0, dispTickMs = 0.0, dispFps = 0.0;

	lastFrame = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window)){
		// ============================== FRAME TIMING ==============================
		// Total wall-clock time the *previous* frame took (frame-to-frame).
		// deltaTime is the master clock every other phase is measured against; a
		// frame's full duration is only knowable once the frame has ended.
		double frameStart = glfwGetTime();
		deltaTime = static_cast<float>(frameStart - lastFrame);
		frameMs   = (frameStart - lastFrame) * 1000.0;
		lastFrame = static_cast<float>(frameStart);

		// ================================= UPDATE =================================
		// Advances the state of the world
		double updateStart = glfwGetTime();
		
		// --- fixed-timestep simulation (TICKS) ---
		// Deterministic game logic (physics, gameplay, sim-driven animation)
		// Runs 0..N times per frame at a constant dt (FIXED_DT), so the simulation
		// is framerate-independent. Always integrate with FIXED_DT, never deltaTime.
		accumulator += deltaTime;
		while (accumulator >= FIXED_DT) {
			double tickStart = glfwGetTime();

			simTime += FIXED_DT;                  // advance the deterministic sim clock
			lightPos.x = sin(simTime) * 2.0f;     // world state mutated per tick
			lightPos.z = cos(simTime) * 2.0f;

			accumulator -= FIXED_DT;
			accTickMs += (glfwGetTime() - tickStart) * 1000.0;  // summed for averaging
			++tickCount;
		}
		
		// --- per-frame input ---
		// Polled once per rendered frame (not per tick)
		processInput(window);

		updateMs = (glfwGetTime() - updateStart) * 1000.0;
		
		// ================================= RENDER =================================
		// Turns the current world state into pixels. Runs once per frame, as fast
		// as the display/vsync allows, and must not mutate simulation state.
		double renderStart = glfwGetTime();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- 3D scene ---
		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		// light properties
		lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		lightingShader.setFloat("material.shininess", 64.0f);

		// camera/view transformations
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		cube.draw();

		// draw the lamp too
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", model);

		cube.draw();

		renderMs = (glfwGetTime() - renderStart) * 1000.0;  // 3D scene only (HUD excluded)

		// ============================== HUD SAMPLING ==============================
		hudController.update(FrameMetrics{deltaTime, frameMs, updateMs, renderMs, accTickMs, tickCount});

		// =============================== HUD / OVERLAY ============================
		// 2D debug readout, toggled with F3. Drawn last so it composites on top of the
		// 3D scene, with depth testing off so geometry can never occlude the text.
		hudController.draw(glm::vec3(1.0f));

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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
}

// discrete key events (edge-triggered): fires once per press, so it's the right
// tool for toggles — unlike glfwGetKey polling in processInput, which repeats while held
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		hudController.toggle();
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

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
		
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
