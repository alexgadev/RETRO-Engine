#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "shader.h"

#include <string>
#include <map>

class TextRenderer
{
    public:
        TextRenderer(const unsigned int scr_width, const unsigned int scr_height,
                        const char* path, int pixelSize = 48);
        ~TextRenderer();

        TextRenderer(const TextRenderer&) = delete;
        TextRenderer& operator=(const TextRenderer&) = delete;
        TextRenderer(TextRenderer&& other) noexcept;
        TextRenderer& operator=(TextRenderer&& other) noexcept;

        
        void drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) const;
        float measureText(const std::string& text, float scale) const;
        float getAscent(float scale) const;
        float getDescent(float scale) const;
        void setProjection();

    private:
        struct Character
        {
            unsigned int TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            unsigned int Advance;
        };
        std::map<GLchar, Character> Characters;
        int glyphMaxAscent  = 0;   // px above the baseline of the tallest glyph in the atlas
        int glyphMaxDescent = 0;   // px below the baseline of the lowest glyph in the atlas

        unsigned int m_vao = 0, m_vbo = 0;

        Shader m_shader;

        void release(); // not const because it releases the objects linked to m_vao and m_vbo, 
					    // not inherently modifying them but modifying what they are linked to

        void loadFont(const char* path, int pixelSize);
};