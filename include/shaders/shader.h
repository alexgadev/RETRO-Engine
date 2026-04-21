#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

class Shader
{
public: 
    unsigned int ID;

    // constructor, reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            std::stringstream vShaderStream, fShaderStream;

            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // close file handlers
            vShaderFile.close();
            fShaderFile.close();

            // convert stream into string
            vertexCode  = vShaderStream.str();
            fragmentCode  = fShaderStream.str();
        }
        catch(std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex, fragment;
	    int success;
	    char infoLog[512];
        
        vertex = glCreateShader(GL_VERTEX_SHADER);
	    // vertexShader ID, nº of strings, array of source code of the shader, array of string lengths
	    glShaderSource(vertex, 1, &vShaderCode, NULL);
	    glCompileShader(vertex);

	    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	    if (!success)
        {
	    	glGetShaderInfoLog(vertex, 512, NULL, infoLog);
	    	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	    }

	    // --- fragment shader ---
	    fragment = glCreateShader(GL_FRAGMENT_SHADER);
	    glShaderSource(fragment, 1, &fShaderCode, NULL);
	    glCompileShader(fragment);

	    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	    if (!success)
        {
	    	glGetShaderInfoLog(fragment, 512, NULL, infoLog);
	    	std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	    }

        // --- shader program ---
        ID = glCreateProgram();
	    glAttachShader(ID, vertex);
	    glAttachShader(ID, fragment);
	    glLinkProgram(ID);

	    glGetProgramiv(ID, GL_LINK_STATUS, &success);
	    if (!success)
        {
	    	glGetProgramInfoLog(ID, 512, NULL, infoLog);
	    	std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	    }

        // we no longer need the shaders as they're already linked into the program
	    glDeleteShader(vertex);
	    glDeleteShader(fragment);
    }

    // use/activate the shader
    void use()
    {
        glUseProgram(ID);
    }

    // utility functions
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
};

#endif