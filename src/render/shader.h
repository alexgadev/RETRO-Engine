#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader
{
public: 
    // constructor, reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // forbid copy constructor and copy assignment
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // use/activate the shader
    void use() const;

    // utility functions
    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setFloat(const std::string &name, float value) const;

    void setVec2(const std::string &name, const glm::vec2 &value) const;

    void setVec2(const std::string &name, float x, float y) const;

    void setVec3(const std::string &name, const glm::vec3 &value) const;

    void setVec3(const std::string &name, float x, float y, float z) const;

    void setVec4(const std::string &name, const glm::vec4 &value) const;

    void setVec4(const std::string &name, float x, float y, float z, float w) const;

    void setMat2(const std::string &name, const glm::mat2 &mat) const;

    void setMat3(const std::string &name, const glm::mat3 &mat) const;

    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void release(); // not const because it releases the objects linked to ID, 
					// not inherently modifying them but modifying what they are linked to

    std::string parseShader(const char* path) const;

    // abstraction of the compilation/linking error check
    void checkCompileErrors(unsigned int shader, const std::string& type) const;

    unsigned int ID = 0;
};