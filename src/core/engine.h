#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "window.h"
#include "../render/hud.h"
#include "../render/camera.h"


// this exists only to guarantee teardown order
struct GlfwTerminator
{
    ~GlfwTerminator() { glfwTerminate(); }
};

class Engine
{
    public:
        Engine(const char* title, unsigned int width, unsigned int height);
        virtual ~Engine() = default;

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;  // owns callback pointers into itself, moving would dangle them
        Engine& operator=(Engine&&) = delete;
    
        // runs the whole frame loop until the window closes
        void run();

    protected:
        // user hooks. note: override only what's needed
        virtual void onInit() {}                    // build shaders, meshes and textures
        virtual void onUpdate(double fixedDt) {}    // fixed-timestep simulation
        virtual void onRender() {}                  // per-frame draw calls

        // what's needed in a scene to draw itself
        Camera& camera();
        const WindowHandler& window() const;
        float deltaTime() const;
        // matrices
        glm::mat4 projectionMatrix() const;
        glm::mat4 viewMatrix() const;

    private:
        void processInput();

        // helper functions called inside the initializer list. both are hard-exits in case of wrong initialization
        static bool initGlfw();
        static bool initGlad();

        // all the GLFW function callbacks that were declared on main
        static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
        static void onFramebufferSize(GLFWwindow* window, int width, int height);

        // the statics above only get a GLFWwindow*, they fetch the Engine from the window user pointer and forward to these
        void handleKey(int key, int action);
        void handleMouseMove(float xpos, float ypos);
        void handleResize(int width, int height);

        // declaration order matters!!!
        GlfwTerminator m_terminator;
        bool m_glfwReady;
        WindowHandler m_windowController;
        bool m_gladReady;
        HUD m_hudController;
        Camera m_camera;

        // timing variables port
        static constexpr double TICK_RATE = 60.0;               // simulation ticks per second
	    static constexpr double FIXED_DT  = 1.0 / TICK_RATE;    // seconds advanced by one tick
	    double m_accumulator = 0.0;                             // unsimulated real time carried between frames
	    double m_simTime     = 0.0;                             // deterministic sim clock (advanced only by ticks)
        float m_deltaTime    = 0.0f;                            // time between current frame and last frame
        float m_lastFrame    = 0.0f;

        // mouse variables port
        float m_lastX = 0.0f;
        float m_lastY = 0.0f;
        bool m_firstMouse = true;
};