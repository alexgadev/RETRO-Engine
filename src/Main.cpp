#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/engine.h"
#include "render/shader.h"
#include "render/mesh.h"

#include <iostream>
#include <cmath>

unsigned int loadTexture(const char *path)
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
};

class BasicScene : public Engine
{
	public:
		BasicScene(const char* title, unsigned int width, unsigned int height)
			: Engine(title, width, height),
			  m_lightingShader("shaders/shader.vs", "shaders/color_shader.fs"),
			  m_lightCubeShader("shaders/light_cube.vs", "shaders/lightcube_shader.fs"),
			  m_cube(Mesh::createCube())
		{}
	
	protected:
		void onInit() override
		{
			m_diffuseMap = loadTexture("resources/assets/container2.png");
			m_specularMap = loadTexture("resources/assets/container2_specular.png");

			m_lightingShader.use();
			m_lightingShader.setInt("material.diffuse", 0);
			m_lightingShader.setInt("material.specular", 1);
		}

		void onUpdate(double fixedDt) override
		{
			m_time += fixedDt;
			m_lightPos.x = sin(m_time) * 2.0f;     // world state mutated per tick
			m_lightPos.z = cos(m_time) * 2.0f;
		}

		void onRender() override
		{
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// --- 3D scene ---
			m_lightingShader.use();
			m_lightingShader.setVec3("light.position", m_lightPos);
			m_lightingShader.setVec3("viewPos", camera().Position);

			// light properties
			m_lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
			m_lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
			m_lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

			m_lightingShader.setFloat("material.shininess", 64.0f);

			// camera/view transformations
			glm::mat4 projection = projectionMatrix();
			glm::mat4 view = viewMatrix();
			m_lightingShader.setMat4("projection", projection);
			m_lightingShader.setMat4("view", view);

			glm::mat4 model = glm::mat4(1.0f);
			m_lightingShader.setMat4("model", model);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_diffuseMap);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_specularMap);

			m_cube.draw();

			// draw the lamp too
			m_lightCubeShader.use();
			m_lightCubeShader.setMat4("projection", projection);
			m_lightCubeShader.setMat4("view", view);
			model = glm::mat4(1.0f);
			model = glm::translate(model, m_lightPos);
			model = glm::scale(model, glm::vec3(0.2f));
			m_lightCubeShader.setMat4("model", model);

			m_cube.draw();
		}

	private:
		Shader m_lightingShader;
		Shader m_lightCubeShader;
		Mesh m_cube;

		unsigned int m_diffuseMap = 0;
		unsigned int m_specularMap = 0;

		glm::vec3 m_lightPos {1.2f, 1.0f, 2.0f};
		double m_time = 0.0;
};

int main(void)
{
	BasicScene scene("Hello", 800, 600);
	scene.run();
	return 0;
}