#include "hud.h"
#include "text_renderer.h"

#include <glad/glad.h>


HUD::HUD(unsigned int scr_width, unsigned int scr_height, const char* textFontPath, const char* textVertexShader, const char* textFragmentShader, const char* bgVertexShader, const char* bgFragmentShader, int pixelSize)
    :   m_text_renderer(scr_width, scr_height, textFontPath, textVertexShader, textFragmentShader, pixelSize),
        m_shader(bgVertexShader, bgFragmentShader),
        m_scr_width(scr_width), m_scr_height(scr_height)
{
    glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &m_quadTex);
	glBindTexture(GL_TEXTURE_2D, m_quadTex);
	
    unsigned char whitePixel = 255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &whitePixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
    
    setProjection(m_scr_width, m_scr_height);
}

HUD::~HUD() { release(); }

HUD::HUD(HUD&& o) noexcept
    : m_shader(std::move(o.m_shader)),
      m_text_renderer(std::move(o.m_text_renderer)),
      m_scale(o.m_scale), m_step(o.m_step), m_alpha(o.m_alpha),
      bgFps(o.bgFps), bgFrame(o.bgFrame), bgUpdate(o.bgUpdate), bgRender(o.bgRender), bgTick(o.bgTick),
      m_scr_width(o.m_scr_width), m_scr_height(o.m_scr_height),
      m_vao(o.m_vao), m_vbo(o.m_vbo), m_quadTex(o.m_quadTex)
{
    o.m_vao = o.m_vbo = o.m_quadTex = 0;
}

HUD& HUD::operator=(HUD&& o) noexcept
{
    if(this != &o)
    {
        release();
        m_scale = o.m_scale; m_step = o.m_step; m_alpha = o.m_alpha;
        bgFps = o.bgFps; bgFrame = o.bgFrame; bgUpdate = o.bgUpdate; bgRender = o.bgRender; bgTick = o.bgTick;
        m_scr_width = o.m_scr_width; m_scr_height = o.m_scr_height;
        m_vao = o.m_vao; m_vbo = o.m_vbo; m_quadTex = o.m_quadTex;
        o.m_vao = o.m_vbo = o.m_quadTex = 0;
        m_shader = std::move(o.m_shader);
        m_text_renderer = std::move(o.m_text_renderer);
    }
    return *this;
}

void HUD::release()
{
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_quadTex) glDeleteBuffers(1, &m_quadTex);
}

void HUD::update(FrameMetrics metrics)
{
    // Collect this frame's numbers, then only recompute the displayed averages
	// every HUD_REFRESH seconds so the text on screen stays readable.
	accFrameMs  += metrics.frameMs;
	accUpdateMs += metrics.updateMs;
	accRenderMs += metrics.renderMs;
    accTickMs = metrics.accTickMs;
    tickCount = metrics.tickCount;
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
		drawTextWithBg("FPS: " + ss.str(), 1.0f, m_scr_height - m_step * 1, m_scale, textColor, bgFps, m_alpha);
		
        ss.str(""); ss << std::setprecision(3) << dispFrameMs;
		drawTextWithBg("FRAME: " + ss.str() + " ms", 1.0f, m_scr_height - m_step * 2, m_scale, textColor, bgFrame, m_alpha);
		
        ss.str(""); ss << dispUpdateMs;
		drawTextWithBg("UPDATE: " + ss.str() + " ms", 1.0f, m_scr_height - m_step * 3, m_scale, textColor, bgUpdate, m_alpha);
		
        ss.str(""); ss << dispRenderMs;
		drawTextWithBg("RENDER: " + ss.str() + " ms", 1.0f, m_scr_height - m_step * 4, m_scale, textColor, bgRender, m_alpha);
		
        ss.str(""); ss << dispTickMs;
		drawTextWithBg("TICK: " + ss.str() + " ms", 1.0f, m_scr_height - m_step * 5, m_scale, textColor, bgTick, m_alpha);
		
        glEnable(GL_DEPTH_TEST);
	}
}

// Draws a single solid-colored quad by reusing the text shader with a white texture.
// alpha < 1.0 makes it translucent (the shader multiplies it into the output alpha).
void HUD::drawQuad(float x, float y, float w, float h, glm::vec3 color, float alpha) const
{
    m_shader.use();
	m_shader.setVec3("color", color.x, color.y, color.z);
	m_shader.setFloat("alpha", alpha);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_quadTex);
    glBindVertexArray(m_vao);

    // same vec4(pos.xy, uv) layout the text VBO expects; uv is irrelevant for a flat color
    float vertices[6][4] = {
        { x,     y + h,   0.0f, 0.0f },
        { x,     y,       0.0f, 1.0f },
        { x + w, y,       1.0f, 1.0f },

        { x,     y + h,   0.0f, 0.0f },
        { x + w, y,       1.0f, 1.0f },
        { x + w, y + h,   1.0f, 0.0f }
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Draws text on top of a solid box. The box width fits the run; its height matches
// the tallest glyph in the atlas, so every label is the same height regardless of text.
void HUD::drawTextWithBg(const std::string& text, float x, float y, float scale, glm::vec3 textColor, glm::vec3 bgColor, float bgAlpha) const
{
    // measure the run width (sum of pen advances, spacing included)
    float w = m_text_renderer.measureText(text, scale);

    // uniform box height = full atlas extent (tallest ascent + deepest descent)
    float hpad   = 2.0f;
    float top    = m_text_renderer.getAscent(scale)  * scale;   // reaches above the baseline
    float bottom = m_text_renderer.getDescent(scale) * scale;   // reaches below the baseline

    // box first (behind), then the glyphs on top
    drawQuad(x - hpad, y - bottom, w + 2.0f * hpad, top + bottom, bgColor, bgAlpha);
    m_text_renderer.drawText(text, x, y, scale, textColor);
}

void HUD::setScale(float scale) { m_scale = scale; }
void HUD::setStep(float scale) { m_scale = scale; }
void HUD::setAlpha(float scale) { m_scale = scale; }
void HUD::setBgFpsColor(glm::vec3 color) { bgFps = color; }
void HUD::setBgFrameColor(glm::vec3 color) { bgFrame = color; }
void HUD::setBgUpdateColor(glm::vec3 color) { bgUpdate = color; }
void HUD::setBgRenderColor(glm::vec3 color) { bgRender = color; }
void HUD::setBgTickColor(glm::vec3 color) { bgTick = color; }