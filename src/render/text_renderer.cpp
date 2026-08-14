#include "text_renderer.h"

#include <GL/gl.h>
#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>


TextRenderer::TextRenderer(unsigned int scr_width, unsigned int scr_height, const char* path, const char* vertexShader, const char* fragmentShader, int pixelSize)
    : m_shader(vertexShader, fragmentShader)
{
    loadFont(path, pixelSize);

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    setProjection(scr_height, scr_height);
}

TextRenderer::~TextRenderer() { release(); }

TextRenderer::TextRenderer(TextRenderer&& o) noexcept
    : Characters(std::move(o.Characters)), 
      m_shader(std::move(o.m_shader)),
      glyphMaxAscent(o.glyphMaxAscent),
      glyphMaxDescent(o.glyphMaxDescent),
      m_vao(o.m_vao), m_vbo(o.m_vbo)
{
    o.m_vao = o.m_vbo = 0;
}

TextRenderer& TextRenderer::operator=(TextRenderer&& o) noexcept
{
    if(this != &o){
        release();
        m_vao = o.m_vao; m_vbo = o.m_vbo; glyphMaxAscent = o.glyphMaxAscent; glyphMaxDescent = o.glyphMaxDescent;
        o.m_vao = o.m_vbo = o.glyphMaxAscent = o.glyphMaxDescent = 0;
        Characters = std::move(o.Characters);
        m_shader = std::move(o.m_shader);
    }
    return *this;
}

void TextRenderer::release()
{
    for(auto& kv : Characters){
        glDeleteTextures(1, &kv.second.TextureID);
    }
    Characters.clear();

    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void TextRenderer::loadFont(const char* path, int pixelSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    
    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, pixelSize);
    
        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
    
        // record the atlas' vertical extents so HUD boxes can share one uniform height
        for (auto &kv : Characters)
        {
            glyphMaxAscent  = std::max(glyphMaxAscent,  kv.second.Bearing.y);
            glyphMaxDescent = std::max(glyphMaxDescent, kv.second.Size.y - kv.second.Bearing.y);
        }
    
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) const
{
    // activate corresponding render state	
    shader.use();
	shader.setVec3("textColor", color.x, color.y, color.z);
	shader.setFloat("alphaMul", 1.0f);   // glyphs: alpha comes from coverage
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float TextRenderer::measureText(const std::string& text, float scale) const
{
    float w = 0.0f;
    for (char c : text)
        w += (Characters[c].Advance >> 6) * scale;
    return w;
}

float TextRenderer::getAscent(float scale) const
{
    return glyphMaxAscent * scale;
}

float TextRenderer::getDescent(float scale) const
{
    return glyphMaxDescent * scale;
}

void TextRenderer::setProjection(unsigned int width, unsigned int height)
{
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) width / height, 0.1f, 100.0f);
	m_shader.setMat4("projection", projection);
}