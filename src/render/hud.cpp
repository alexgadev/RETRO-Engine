#include "hud.h"
#include "text_renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <sstream>
#include <iomanip>


HUD::HUD(unsigned int scr_width, unsigned int scr_height, const char* textFontPath, const char* textVertexShader, const char* textFragmentShader, const char* bgVertexShader, const char* bgFragmentShader, int pixelSize)
    :   m_shader(bgVertexShader, bgFragmentShader),
        m_text_renderer(scr_width, scr_height, textFontPath, textVertexShader, textFragmentShader, pixelSize),
        m_scr_width(scr_width), m_scr_height(scr_height)
{
    glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    
    setProjection(m_scr_width, m_scr_height);
}

HUD::~HUD() { release(); }

HUD::HUD(HUD&& o) noexcept
    : m_shader(std::move(o.m_shader)),
      m_text_renderer(std::move(o.m_text_renderer)),
      m_scr_width(o.m_scr_width), m_scr_height(o.m_scr_height),
      m_scale(o.m_scale), m_step(o.m_step), m_alpha(o.m_alpha),
      bgFps(o.bgFps), bgFrame(o.bgFrame), bgUpdate(o.bgUpdate), bgRender(o.bgRender), bgTick(o.bgTick),
      m_vao(o.m_vao), m_vbo(o.m_vbo)
{
    o.m_vao = o.m_vbo = 0;
}

HUD& HUD::operator=(HUD&& o) noexcept
{
    if(this != &o)
    {
        release();
        m_scale = o.m_scale; m_step = o.m_step; m_alpha = o.m_alpha;
        bgFps = o.bgFps; bgFrame = o.bgFrame; bgUpdate = o.bgUpdate; bgRender = o.bgRender; bgTick = o.bgTick;
        m_scr_width = o.m_scr_width; m_scr_height = o.m_scr_height;
        m_vao = o.m_vao; m_vbo = o.m_vbo;
        o.m_vao = o.m_vbo = 0;
        m_shader = std::move(o.m_shader);
        m_text_renderer = std::move(o.m_text_renderer);
    }
    return *this;
}

void HUD::release()
{
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void HUD::update(FrameMetrics metrics)
{
    // Collect this frame's numbers, then only recompute the displayed averages
	// every HUD_REFRESH seconds so the text on screen stays readable.
	accFrameMs  += metrics.frameMs;
	accUpdateMs += metrics.updateMs;
	accRenderMs += metrics.renderMs;
    accTickMs += metrics.accTickMs;
    tickCount += metrics.tickCount;
	++frameCount;
	hudTimer += metrics.deltaTime;
	if (hudTimer >= HUD_REFRESH) {
		dispFrameMs  = accFrameMs  / frameCount;
		dispUpdateMs = accUpdateMs / frameCount;
		dispRenderMs = accRenderMs / frameCount;
		dispTickMs   = tickCount ? accTickMs / tickCount : 0.0;
		dispFps      = frameCount / hudTimer;   // avg frames per second this window
		accFrameMs = accUpdateMs = accRenderMs = accTickMs = 0.0;
		frameCount = tickCount = 0;
		hudTimer   = 0.0;
	}
}

void HUD::toggle()
{
    showHud = !showHud;
}

void HUD::draw(glm::vec3 textColor) const
{
    if (showHud)
	{
		glDisable(GL_DEPTH_TEST);
		
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << dispFps;
		drawTextWithBg("FPS: " + ss.str(), 1.0f, static_cast<float>(m_scr_height) - m_step * 1, m_scale, textColor, bgFps, m_alpha);
		
        ss.str(""); ss << std::setprecision(3) << dispFrameMs;
		drawTextWithBg("FRAME: " + ss.str() + " ms", 1.0f, static_cast<float>(m_scr_height) - m_step * 2, m_scale, textColor, bgFrame, m_alpha);
		
        ss.str(""); ss << dispUpdateMs;
		drawTextWithBg("UPDATE: " + ss.str() + " ms", 1.0f, static_cast<float>(m_scr_height) - m_step * 3, m_scale, textColor, bgUpdate, m_alpha);
		
        ss.str(""); ss << dispRenderMs;
		drawTextWithBg("RENDER: " + ss.str() + " ms", 1.0f, static_cast<float>(m_scr_height) - m_step * 4, m_scale, textColor, bgRender, m_alpha);
		
        ss.str(""); ss << dispTickMs;
		drawTextWithBg("TICK: " + ss.str() + " ms", 1.0f, static_cast<float>(m_scr_height) - m_step * 5, m_scale, textColor, bgTick, m_alpha);
		
        glEnable(GL_DEPTH_TEST);
	}
}

// Draws a single solid-colored quad
// alpha < 1.0 makes it translucent (the shader multiplies it into the output alpha).
void HUD::drawQuad(float x, float y, float w, float h, glm::vec3 color, float alpha) const
{
    m_shader.use();
	m_shader.setVec3("color", color.x, color.y, color.z);
	m_shader.setFloat("alpha", alpha);
    glBindVertexArray(m_vao);

    // same vec2(pos.xy) layout the text VBO expects; 
    float vertices[6][2] = {
        { x,     y + h },
        { x,     y },
        { x + w, y },

        { x,     y + h },
        { x + w, y },
        { x + w, y + h }
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}

// Draws text on top of a solid box. The box width fits the run; its height matches
// the tallest glyph in the atlas, so every label is the same height regardless of text.
void HUD::drawTextWithBg(const std::string& text, float x, float y, float scale, glm::vec3 textColor, glm::vec3 bgColor, float bgAlpha) const
{
    // measure the run width (sum of pen advances, spacing included)
    float w = m_text_renderer.measureText(text, scale);

    // uniform box height = full atlas extent (tallest ascent + deepest descent)
    float hpad   = 2.0f;
    float top    = m_text_renderer.getAscent(scale);   // reaches above the baseline
    float bottom = m_text_renderer.getDescent(scale);   // reaches below the baseline

    // box first (behind), then the glyphs on top
    drawQuad(x - hpad, y - bottom, w + 2.0f * hpad, top + bottom, bgColor, bgAlpha);
    m_text_renderer.drawText(text, x, y, scale, textColor);
}

void HUD::setScale(float scale) { m_scale = scale; }
void HUD::setStep(float step) { m_step = step; }
void HUD::setAlpha(float alpha) { m_alpha = alpha; }
void HUD::setBgFpsColor(glm::vec3 color) { bgFps = color; }
void HUD::setBgFrameColor(glm::vec3 color) { bgFrame = color; }
void HUD::setBgUpdateColor(glm::vec3 color) { bgUpdate = color; }
void HUD::setBgRenderColor(glm::vec3 color) { bgRender = color; }
void HUD::setBgTickColor(glm::vec3 color) { bgTick = color; }

void HUD::setProjection(unsigned int width, unsigned int height)
{
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    m_shader.use();
	m_shader.setMat4("projection", projection);
}