#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <array> // fixed size arrays
#include <cstdint>

class Cube
{
public:
	Cube();
	~Cube();

	// avoid generating a copy constructor / copy-assignment operator
	// (useful so that there are no two different instances of cube with
	//  same integer IDs definitions corresponding to VAOs etc)
	Cube(const Cube&) = delete;
	Cube& operator=(const Cube&) = delete;
    Cube(Cube&& other) noexcept;
	Cube& operator=(Cube&& other) noexcept;

    void draw() const;

private:
	void upload();
	void release();

	GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;

	// pos(3) normal(3) uv(2) per vertex, 8 unique corners
	static constexpr std::array<float, 8 * 8> s_vertices = {
		-0.5f,-0.5f,-0.5f,  0,0,-1,  0,0,
        0.5f,-0.5f,-0.5f,  0,0,-1,  1,0,
    	0.5f, 0.5f,-0.5f,  0,0,-1,  1,1,
        -0.5f, 0.5f,-0.5f,  0,0,-1,  0,1,
        -0.5f,-0.5f, 0.5f,  0,0, 1,  0,0,
         0.5f,-0.5f, 0.5f,  0,0, 1,  1,0,
         0.5f, 0.5f, 0.5f,  0,0, 1,  1,1,
        -0.5f, 0.5f, 0.5f,  0,0, 1,  0,1,
	};

	static constexpr std::array<std::uint32_t, 36> s_indices = {
		0,1,2, 2,3,0,       // back
        4,5,6, 6,7,4,       // front
        4,0,3, 3,7,4,       // left
        1,5,6, 6,2,1,       // right
        0,1,5, 5,4,0,       // bottom
        3,2,6, 6,7,3        // top
	};
};