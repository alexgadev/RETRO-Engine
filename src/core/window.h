#pragma once

#include <GLFW/glfw3.h>

class WindowHandler
{
    public:
        WindowHandler(const char* title, unsigned int width = 800, unsigned int height = 600);
        ~WindowHandler();

        WindowHandler(const WindowHandler&) = delete;
        WindowHandler& operator=(const WindowHandler&) = delete;
        WindowHandler(WindowHandler&& other) noexcept;
        WindowHandler& operator=(WindowHandler&& other) noexcept;
        
        bool isValid() const;

        GLFWwindow* getHandle() const;
        unsigned int getWidth() const;
        unsigned int getHeight() const;

        void setSize(unsigned int width, unsigned int height);

        bool shouldClose() const; 
        void swapBuffers() const;
        void pollEvents() const;
        void close() const;

    private:
        unsigned int m_scr_width;
        unsigned int m_scr_height;

        GLFWwindow* m_window = nullptr;
};