#include "window.h"

#include <iostream>


WindowHandler::WindowHandler(const char* title, unsigned int width, unsigned int height)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_scr_width = width;
    m_scr_height = height;

	m_window = glfwCreateWindow(m_scr_width, m_scr_height, title, NULL, NULL);
	if (!m_window)
	{
		std::cerr << "Failed to create window\n";
		return;
	}

	glfwMakeContextCurrent(m_window);
}

WindowHandler::~WindowHandler()
{
    if(m_window) glfwDestroyWindow(m_window);
}

WindowHandler::WindowHandler(WindowHandler&& o) noexcept
    : m_scr_width(o.m_scr_width), m_scr_height(o.m_scr_height),
      m_window(o.m_window)
{
    o.m_window = nullptr;
}

WindowHandler& WindowHandler::operator=(WindowHandler&& o) noexcept
{
    if(this != &o){
        if(m_window) glfwDestroyWindow(m_window);
        m_scr_width = o.m_scr_width; m_scr_height = o.m_scr_height;
        m_window = o.m_window;
        o.m_window = nullptr;
    }
    return *this;
}

bool WindowHandler::isValid() const { return m_window != nullptr; }
GLFWwindow* WindowHandler::getHandle() const { return m_window; }
unsigned int WindowHandler::getWidth() const { return m_scr_width; }
unsigned int WindowHandler::getHeight() const { return m_scr_height; }

void WindowHandler::setSize(unsigned int width, unsigned int height)
{
    m_scr_width = width;
    m_scr_height = height;    
}

bool WindowHandler::shouldClose() const { return glfwWindowShouldClose(m_window); }
void WindowHandler::swapBuffers() const { glfwSwapBuffers(m_window); }
void WindowHandler::pollEvents() const { glfwPollEvents(); }
void WindowHandler::close() const { glfwSetWindowShouldClose(m_window, true); }