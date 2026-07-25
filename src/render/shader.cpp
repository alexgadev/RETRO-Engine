#include "shader.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>


Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    try
    {
        std::string vertexCode = parseShader(vertexPath);
        std::string fragmentCode = parseShader(fragmentPath);
        
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        
        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        // vertexShader ID, nº of strings, array of source code of the shader, array of string lengths
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        
        // --- fragment shader ---
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        // --- shader program ---
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        
        // we no longer need the shaders as they're already linked into the program
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    catch(std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
    
}

Shader::~Shader() { release(); }

Shader::Shader(Shader&& o) noexcept
    : ID(o.ID)
{
    o.ID = 0;
}

Shader& Shader::operator=(Shader&& o) noexcept
{
    if(this != &o){
        release();
        ID = o.ID;
        o.ID = 0;
    }
    return *this;
}

void Shader::release()
{
    if(ID) glDeleteProgram(ID);
}

std::string Shader::parseShader(const char* path) const
{
    std::ifstream shaderFile;
    
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    
    // open file
    shaderFile.open(path);
    
    std::stringstream shaderStream;
    
    // read file's buffer contents into streams
    shaderStream << shaderFile.rdbuf();
    
    // close file handlers
    shaderFile.close();
    
    // convert stream into string
    return shaderStream.str();
}

// use/activate the shader
void Shader::use() const
{
    glUseProgram(ID);
}

// utility functions
void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(unsigned int shader, const std::string& type) const
{
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: \n" << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: \n" << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}