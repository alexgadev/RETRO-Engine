#include "mesh.h"

#include <cstdint>
#include <glad/glad.h> // holds all OpenGL type declarations
#include <array> // fixed size arrays

Mesh::Mesh(const std::vector<float>& s_vertices, const std::vector<uint32_t>& s_indices){
    upload(s_vertices, s_indices);    
}

Mesh::~Mesh() { release(); }

Mesh::Mesh(Mesh&& o) noexcept
    : m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo), n_indices(o.n_indices)
{
    o.m_vao = o.m_vbo = o.m_ebo = o.n_indices = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept
{
    if(this != &o){
        release();
        m_vao = o.m_vao; m_vbo = o.m_vbo; m_ebo = o.m_ebo; n_indices = o.n_indices;
        o.m_vao = o.m_vbo = o.m_ebo = o.n_indices = 0;
    }
    return *this;
}

void Mesh::upload(const std::vector<float>& s_vertices, const std::vector<uint32_t>& s_indices)
{
    n_indices = static_cast<GLsizei>(s_indices.size());

    glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

	glBindVertexArray(m_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, s_vertices.size() * sizeof(float), s_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_indices.size() * sizeof(uint32_t), s_indices.data(), GL_STATIC_DRAW);

	constexpr GLsizei stride = 8 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Mesh::release()
{
    if(m_ebo) glDeleteBuffers(1, &m_ebo);
    if(m_vbo) glDeleteBuffers(1, &m_vbo);
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Mesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
}

Mesh Mesh::createCube()
{
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

    Mesh cube(std::vector<float>(s_vertices.begin(), s_vertices.end()), 
                std::vector<uint32_t>(s_indices.begin(), s_indices.end()));
    return cube;
}