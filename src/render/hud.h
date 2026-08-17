#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "text_renderer.h"

struct FrameMetrics
{
    double frameMs;
    double updateMs;
    double renderMs;
    int frameCount;
    float deltaTime;
};

class HUD
{
    public:
        HUD(unsigned int scr_width, unsigned int scr_height, const char* path, const char* vertexShader, const char* fragmentShader, int pixelSize = 48);
        ~HUD();

        HUD(const HUD&) = delete;
        HUD& operator=(const HUD&) = delete;
        HUD(HUD&& other) noexcept;
        HUD& operator=(HUD&& other) noexcept;

        void update(FrameMetrics metrics);
        void toggle();
        void draw();

        void DrawQuad(float x, float y, float w, float h, glm::vec3 color, float alpha);
        void DrawTextWithBgColor();
        

        TextRenderer m_text_renderer;
    
    private:
        unsigned int m_vao = 0, m_vbo = 0;
        bool showHud = false;
};
