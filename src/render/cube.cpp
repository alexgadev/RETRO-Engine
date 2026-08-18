#include "cube.h"

#include <glad/glad.h>

Cube::Cube() { upload(); }
Cube::~Cube() { release(); }

Cube::Cube(Cube&& o) noexcept 
    : m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo)
{
    o.m_vao = o.m_vbo = o.m_ebo = 0;
}

Cube& Cube::operator=(Cube&& o) noexcept
{
    if(this != &o){
        release();
        m_vao = o.m_vao; m_vbo = o.m_vbo; m_ebo = o.m_ebo;
        o.m_vao = o.m_vbo = o.m_ebo = 0;
    }
    return *this;
}

void Cube::upload()
{
    glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

	glBindVertexArray(m_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertices), s_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_indices), s_indices.data(), GL_STATIC_DRAW);

	constexpr GLsizei stride = 8 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Cube::release()
{
    if(m_ebo) glDeleteBuffers(1, &m_ebo);
    if(m_vbo) glDeleteBuffers(1, &m_vbo);
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Cube::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}