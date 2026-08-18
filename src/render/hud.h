#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "text_renderer.h"

#include <string>

struct FrameMetrics
{
    float deltaTime;
    double frameMs;
    double updateMs;
    double renderMs;
    double accTickMs;
    int tickCount;
};

class HUD
{
    public:
        HUD(unsigned int scr_width, unsigned int scr_height, 
            const char* textFontPath, 
            const char* textVertexShader, const char* textFragmentShader, 
            const char* bgVertexShader, const char* bgFragmentShader,
            int pixelSize = 48);
        ~HUD();

        HUD(const HUD&) = delete;
        HUD& operator=(const HUD&) = delete;
        HUD(HUD&& other) noexcept;
        HUD& operator=(HUD&& other) noexcept;

        void update(FrameMetrics metrics);
        void toggle();
        void draw(glm::vec3 textColor) const;

        void drawQuad(float x, float y, float w, float h, glm::vec3 color, float alpha) const;
        void drawTextWithBg(const std::string& text, float x, float y, float scale, glm::vec3 textColor, glm::vec3 bgColor, float alpha) const;
        
        void setScale(float scale);
        void setStep(float step);
        void setAlpha(float alpha);
        void setBgFpsColor(glm::vec3 color);
        void setBgFrameColor(glm::vec3 color);
        void setBgUpdateColor(glm::vec3 color);
        void setBgRenderColor(glm::vec3 color);
        void setBgTickColor(glm::vec3 color);
    private:
        unsigned int m_vao = 0, m_vbo = 0, m_quadTex;
        bool showHud = true;
        
        Shader m_shader;

        TextRenderer m_text_renderer;
        
        static unsigned int m_scr_height, m_scr_width;

        // Accumulators
        double hudTimer = 0.0, accFrameMs = 0.0, accUpdateMs = 0.0, accRenderMs = 0.0, accTickMs = 0.0;
        int    frameCount = 0, tickCount = 0;
        // Display values
        double dispFrameMs = 0.0, dispUpdateMs = 0.0, dispRenderMs = 0.0, dispTickMs = 0.0, dispFps = 0.0;
        // Config
        static constexpr double HUD_REFRESH = 0.25;    // seconds between on-screen text updates
        float m_scale = 0.25f;                // text scale
        float m_step = 16.0f;                 // vertical spacing between lines
        float m_alpha = 1.0f;                 // background opacity

        // Background colors
        glm::vec3 bgFps   {0.90f, 0.52f, 0.15f};  // amber
		glm::vec3 bgFrame {0.11f, 0.47f, 0.30f};  // emerald green
		glm::vec3 bgUpdate{0.20f, 0.40f, 0.75f};  // cobalt blue
		glm::vec3 bgRender{0.78f, 0.22f, 0.28f};  // crimson red
		glm::vec3 bgTick  {0.52f, 0.26f, 0.64f};  // amethyst purple


        void release();
};
