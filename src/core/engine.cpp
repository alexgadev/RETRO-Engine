#include "engine.h"
#include "window.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>

#include <iostream>

Engine::Engine(const char* title, unsigned int width, unsigned int height)
    : m_glfwReady(initGlfw()),
      m_windowController(title, width, height),
      m_gladReady(initGlad()),
      m_hudController(width, height, "resources/fonts/Antonio-Bold.ttf", 
						"shaders/textShader.vs", "shaders/textShader.fs",
						"shaders/hudShader.vs", "shaders/hudShader.fs"),
      m_camera(glm::vec3(0.0f, 0.0f, 3.0f))
{
    GLFWwindow* handle = m_windowController.getHandle();

    glfwSetCursorPosCallback(handle, onMouseMove);
	glfwSetFramebufferSizeCallback(handle, onFramebufferSize);
	glfwSetKeyCallback(handle, onKey);

    glfwSetWindowUserPointer(handle, this);

    // tell GLFW to capture our mouse
	glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Engine::run()
{
    onInit();

    while (!m_windowController.shouldClose()){
		// ---- HUD readout: averaged over a fixed window so the numbers hold still
		//      long enough to read (raw per-frame values change too fast) ----
		double accTickMs = 0.0;
		int    tickCount = 0;

		// ============================== FRAME TIMING ==============================
		// Total wall-clock time the *previous* frame took (frame-to-frame).
		// deltaTime is the master clock every other phase is measured against; a
		// frame's full duration is only knowable once the frame has ended.
		double frameStart = glfwGetTime();
		m_deltaTime = static_cast<float>(frameStart - m_lastFrame);
		double frameMs   = (frameStart - m_lastFrame) * 1000.0;
		m_lastFrame = static_cast<float>(frameStart);

		// ================================= UPDATE =================================
		// Advances the state of the world
		double updateStart = glfwGetTime();
		
		// --- fixed-timestep simulation (TICKS) ---
		// Deterministic game logic (physics, gameplay, sim-driven animation)
		// Runs 0..N times per frame at a constant dt (FIXED_DT), so the simulation
		// is framerate-independent. Always integrate with FIXED_DT, never deltaTime.
		m_accumulator += m_deltaTime;
		while (m_accumulator >= FIXED_DT) {
			double tickStart = glfwGetTime();

			m_simTime += FIXED_DT;                  // advance the deterministic sim clock

            onUpdate(FIXED_DT);

			m_accumulator -= FIXED_DT;
			accTickMs += (glfwGetTime() - tickStart) * 1000.0;  // summed for averaging
			++tickCount;
		}
		
		// --- per-frame input ---
		// Polled once per rendered frame (not per tick)
		processInput();

		double updateMs = (glfwGetTime() - updateStart) * 1000.0;
		
		// ================================= RENDER =================================
		// Turns the current world state into pixels. Runs once per frame, as fast
		// as the display/vsync allows, and must not mutate simulation state.
		double renderStart = glfwGetTime();

		onRender();

		double renderMs = (glfwGetTime() - renderStart) * 1000.0;  // 3D scene only (HUD excluded)

		// ============================== HUD SAMPLING ==============================
		m_hudController.update(FrameMetrics{m_deltaTime, frameMs, updateMs, renderMs, accTickMs, tickCount});

		// =============================== HUD / OVERLAY ============================
		// 2D debug readout, toggled with F3. Drawn last so it composites on top of the
		// 3D scene, with depth testing off so geometry can never occlude the text.
		m_hudController.draw(glm::vec3(1.0f));

		// glfw: swap buffers and poll IO events
		m_windowController.swapBuffers();
		m_windowController.pollEvents();
	}
}

Camera& Engine::camera() { return m_camera; }
const WindowHandler& Engine::window() const { return m_windowController; }
float Engine::deltaTime() const { return m_deltaTime; }

glm::mat4 Engine::projectionMatrix() const 
{
    return glm::perspective(glm::radians(45.0f), 
                            static_cast<float>(m_windowController.getWidth()) / 
                            static_cast<float>(m_windowController.getHeight()), 
                            0.1f, 100.0f);
}

glm::mat4 Engine::viewMatrix() const { return m_camera.GetViewMatrix(); }

void Engine::processInput()
{
    if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_windowController.getHandle(), true);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_W) == GLFW_PRESS)
		m_camera.ProcessKeyboard(FORWARD, m_deltaTime);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_S) == GLFW_PRESS)
		m_camera.ProcessKeyboard(BACKWARD, m_deltaTime);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_A) == GLFW_PRESS)
		m_camera.ProcessKeyboard(LEFT, m_deltaTime);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_D) == GLFW_PRESS)
		m_camera.ProcessKeyboard(RIGHT, m_deltaTime);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_SPACE) == GLFW_PRESS)
		m_camera.ProcessKeyboard(UP, m_deltaTime);
	if (glfwGetKey(m_windowController.getHandle(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		m_camera.ProcessKeyboard(DOWN, m_deltaTime);
}

bool Engine::initGlfw()
{
    if (!glfwInit())
	{
		std::cerr << "Failed to init GLFW\n";
		std::exit(-1);
	} 
    return true;
}

bool Engine::initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		std::exit(-1);
	} 
    return true;
}

void Engine::onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if(engine) engine -> handleKey(key, action);
}

void Engine::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if(engine) engine -> handleMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
}

void Engine::onFramebufferSize(GLFWwindow* window, int width, int height)
{
    auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if(engine) engine -> handleResize(width, height);
}

void Engine::handleKey(int key, int action)
{
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
	{
		m_hudController.toggle();
	}
}

void Engine::handleMouseMove(float xpos, float ypos)
{
	if(m_firstMouse)
	{
		m_lastX = xpos;
		m_lastY = ypos;
		m_firstMouse = false;
	}

	float xoffset = xpos - m_lastX;
	float yoffset = m_lastY - ypos;
	
	m_lastX = xpos;
	m_lastY = ypos;

	m_camera.ProcessMouseMovement(xoffset, yoffset);
}

void Engine::handleResize(int width, int height)
{
    glViewport(0, 0, width, height);
}