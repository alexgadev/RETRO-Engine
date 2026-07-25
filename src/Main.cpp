#include <charconv>
#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "render/shader.h"
#include "render/camera.h"
#include "render/cube.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void generate_cube_positions_sb(glm::vec3 cubePositions[]);
unsigned int loadTexture(const char *path);
void RenderText(unsigned int VAO, unsigned int VBO, const Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void RenderQuad(unsigned int VAO, unsigned int VBO, const Shader& shader, unsigned int tex, float x, float y, float w, float h, glm::vec3 color, float alpha);
void RenderTextBg(unsigned int VAO, unsigned int VBO, const Shader& shader, unsigned int whiteTex, const std::string& text, float x, float y, float scale, glm::vec3 textColor, glm::vec3 bgColor, float bgAlpha);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// HUD
bool showHud = true;   // toggled with F3

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main(void){
	if (!glfwInit())
	{
		std::cerr << "Failed to init GLFW\n";
		return -1;
	} 

	// for now I'll be using version 3.3 (LearnOpenGL tutorials' version)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// build and compile the shader program
	Shader lightingShader("shaders/shader.vs", "shaders/color_shader.fs");
	Shader lightCubeShader("shaders/light_cube.vs", "shaders/lightcube_shader.fs");
	Shader textShader("shaders/textShader.vs", "shaders/textShader.fs");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
	textShader.use();
	textShader.setMat4("projection", projection);

	Cube cube; // only one needed, model transformations will enable multiple cubes to be created with the same instance
	
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	FT_Face face;
	if (FT_New_Face(ft, "resources/fonts/Antonio-Bold.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

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
	
	unsigned int textVAO, textVBO;
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 1x1 white texture: sampled as coverage = 1.0 everywhere, so the text shader
	// paints a solid rectangle. Lets RenderQuad reuse textShader for HUD backgrounds.
	unsigned int whiteTex;
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	unsigned char whitePixel = 255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &whitePixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);



	unsigned int diffuseMap = loadTexture("resources/assets/container2.png");
	unsigned int specularMap = loadTexture("resources/assets/container2_specular.png");
	
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	// ---- fixed-timestep config ----
	const double TICK_RATE = 60.0;              // simulation ticks per second
	const double FIXED_DT  = 1.0 / TICK_RATE;   // seconds advanced by one tick
	double accumulator = 0.0;                   // unsimulated real time carried between frames
	double simTime     = 0.0;                   // deterministic sim clock (advanced only by ticks)

	// ---- per-frame metrics (milliseconds), measured every frame ----
	double frameMs = 0.0, updateMs = 0.0, renderMs = 0.0;

	// ---- HUD readout: averaged over a fixed window so the numbers hold still
	//      long enough to read (raw per-frame values change too fast) ----
	const double HUD_REFRESH = 0.25;   // seconds between on-screen text updates
	double hudTimer   = 0.0;          // real time elapsed since the last refresh
	double accFrameMs = 0.0, accUpdateMs = 0.0, accRenderMs = 0.0, accTickMs = 0.0;
	int    frameCount = 0, tickCount = 0;   // samples gathered during this window
	double dispFrameMs = 0.0, dispUpdateMs = 0.0, dispRenderMs = 0.0, dispTickMs = 0.0, dispFps = 0.0;

	lastFrame = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window)){
		// ============================== FRAME TIMING ==============================
		// Total wall-clock time the *previous* frame took (frame-to-frame).
		// deltaTime is the master clock every other phase is measured against; a
		// frame's full duration is only knowable once the frame has ended.
		double frameStart = glfwGetTime();
		deltaTime = static_cast<float>(frameStart - lastFrame);
		frameMs   = (frameStart - lastFrame) * 1000.0;
		lastFrame = static_cast<float>(frameStart);

		// ================================= UPDATE =================================
		// Advances the state of the world
		double updateStart = glfwGetTime();
		
		// --- fixed-timestep simulation (TICKS) ---
		// Deterministic game logic (physics, gameplay, sim-driven animation)
		// Runs 0..N times per frame at a constant dt (FIXED_DT), so the simulation
		// is framerate-independent. Always integrate with FIXED_DT, never deltaTime.
		accumulator += deltaTime;
		while (accumulator >= FIXED_DT) {
			double tickStart = glfwGetTime();

			simTime += FIXED_DT;                  // advance the deterministic sim clock
			lightPos.x = sin(simTime) * 2.0f;     // world state mutated per tick
			lightPos.z = cos(simTime) * 2.0f;

			accumulator -= FIXED_DT;
			accTickMs += (glfwGetTime() - tickStart) * 1000.0;  // summed for averaging
			++tickCount;
		}
		
		// --- per-frame input ---
		// Polled once per rendered frame (not per tick)
		processInput(window);

		updateMs = (glfwGetTime() - updateStart) * 1000.0;
		
		// ================================= RENDER =================================
		// Turns the current world state into pixels. Runs once per frame, as fast
		// as the display/vsync allows, and must not mutate simulation state.
		double renderStart = glfwGetTime();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --- 3D scene ---
		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		// light properties
		lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		lightingShader.setFloat("material.shininess", 64.0f);

		// camera/view transformations
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		cube.draw();

		// draw the lamp too
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", model);

		cube.draw();

		renderMs = (glfwGetTime() - renderStart) * 1000.0;  // 3D scene only (HUD excluded)

		// ============================== HUD SAMPLING ==============================
		// Collect this frame's numbers, then only recompute the displayed averages
		// every HUD_REFRESH seconds so the text on screen stays readable.
		accFrameMs  += frameMs;
		accUpdateMs += updateMs;
		accRenderMs += renderMs;
		++frameCount;
		hudTimer += deltaTime;
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

		// =============================== HUD / OVERLAY ============================
		// 2D debug readout, toggled with F3. Drawn last so it composites on top of the
		// 3D scene, with depth testing off so geometry can never occlude the text.
		if (showHud)
		{
			glDisable(GL_DEPTH_TEST);
			const glm::vec3 fg(1.0f);            // white text
			const float s = 0.25f, a = 1.0f;     // text scale, opaque backgrounds
			const float step = 16.0f;            // vertical spacing between lines
			const glm::vec3 bgFps   (0.90f, 0.52f, 0.15f);  // amber
			const glm::vec3 bgFrame (0.11f, 0.47f, 0.30f);  // emerald green
			const glm::vec3 bgUpdate(0.20f, 0.40f, 0.75f);  // cobalt blue
			const glm::vec3 bgRender(0.78f, 0.22f, 0.28f);  // crimson red
			const glm::vec3 bgTick  (0.52f, 0.26f, 0.64f);  // amethyst purple
			std::stringstream ss;

			ss << std::fixed << std::setprecision(1) << dispFps;
			RenderTextBg(textVAO, textVBO, textShader, whiteTex, "FPS: " + ss.str(), 1.0f, SCR_HEIGHT - step * 1, s, fg, bgFps, a);

			ss.str(""); ss << std::setprecision(3) << dispFrameMs;
			RenderTextBg(textVAO, textVBO, textShader, whiteTex, "FRAME: " + ss.str() + " ms", 1.0f, SCR_HEIGHT - step * 2, s, fg, bgFrame, a);

			ss.str(""); ss << dispUpdateMs;
			RenderTextBg(textVAO, textVBO, textShader, whiteTex, "UPDATE: " + ss.str() + " ms", 1.0f, SCR_HEIGHT - step * 3, s, fg, bgUpdate, a);

			ss.str(""); ss << dispRenderMs;
			RenderTextBg(textVAO, textVBO, textShader, whiteTex, "RENDER: " + ss.str() + " ms", 1.0f, SCR_HEIGHT - step * 4, s, fg, bgRender, a);

			ss.str(""); ss << dispTickMs;
			RenderTextBg(textVAO, textVBO, textShader, whiteTex, "TICK: " + ss.str() + " ms", 1.0f, SCR_HEIGHT - step * 5, s, fg, bgTick, a);

			glEnable(GL_DEPTH_TEST);
		}

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

// discrete key events (edge-triggered): fires once per press, so it's the right
// tool for toggles — unlike glfwGetKey polling in processInput, which repeats while held
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		showHud = !showHud;
}

// whenever the window size changes (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if(firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void generate_cube_positions_sb(glm::vec3 cubePositions[])
{
	int i = 0;

	for(int x = 0; x < 6; ++x)
	{
		for(int z = -2; z < 1; ++z)
		{
			for(int y = 0; y < 3; ++y)
			{
				cubePositions[i++] = glm::vec3((float) x, (float) y, (float) z);
			}
		}
	}

	for(int x = 3; x < 6; ++x)
	{
		for(int z = 1; z < 4; ++z)
		{
			for(int y = 0; y < 3; ++y)
			{
				cubePositions[i++] = glm::vec3((float) x, (float) y, (float) z);
			}
		}
	}
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
		
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void RenderText(unsigned int VAO, unsigned int VBO, const Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
	shader.setVec3("textColor", color.x, color.y, color.z);
	shader.setFloat("alphaMul", 1.0f);   // glyphs: alpha comes from coverage
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

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
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

// Draws a single solid-colored quad by reusing the text shader with a white texture.
// alpha < 1.0 makes it translucent (the shader multiplies it into the output alpha).
void RenderQuad(unsigned int VAO, unsigned int VBO, const Shader& shader, unsigned int tex, float x, float y, float w, float h, glm::vec3 color, float alpha)
{
    shader.use();
	shader.setVec3("textColor", color.x, color.y, color.z);
	shader.setFloat("alphaMul", alpha);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(VAO);

    // same vec4(pos.xy, uv) layout the text VBO expects; uv is irrelevant for a flat color
    float vertices[6][4] = {
        { x,     y + h,   0.0f, 0.0f },
        { x,     y,       0.0f, 1.0f },
        { x + w, y,       1.0f, 1.0f },

        { x,     y + h,   0.0f, 0.0f },
        { x + w, y,       1.0f, 1.0f },
        { x + w, y + h,   1.0f, 0.0f }
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Draws text on top of a solid box. The box width fits the run; its height matches
// the tallest glyph in the atlas, so every label is the same height regardless of text.
void RenderTextBg(unsigned int VAO, unsigned int VBO, const Shader& shader, unsigned int whiteTex, const std::string& text, float x, float y, float scale, glm::vec3 textColor, glm::vec3 bgColor, float bgAlpha)
{
    // measure the run width (sum of pen advances, spacing included)
    float w = 0.0f;
    for (char c : text)
        w += (Characters[c].Advance >> 6) * scale;

    // uniform box height = full atlas extent (tallest ascent + deepest descent)
    float hpad   = 2.0f;
    float top    = glyphMaxAscent  * scale;   // reaches above the baseline
    float bottom = glyphMaxDescent * scale;   // reaches below the baseline

    // box first (behind), then the glyphs on top
    RenderQuad(VAO, VBO, shader, whiteTex, x - hpad, y - bottom, w + 2.0f * hpad, top + bottom, bgColor, bgAlpha);
    RenderText(VAO, VBO, shader, text, x, y, scale, textColor);
}
