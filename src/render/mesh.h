#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <vector>
#include <cstdint>

class Mesh 
{
public:
    Mesh(const std::vector<float>& s_vertices, const std::vector<uint32_t>& s_indices);
    ~Mesh();

    // avoid generating a copy constructor / copy-assignment operator
	// (useful so that there are no two different instances of mesh with
	//  same integer IDs definitions corresponding to VAOs etc)
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&  other) noexcept;
    Mesh& operator=(Mesh&&  other) noexcept;

    void draw() const;

    static Mesh createCube();
    static Mesh createSphere();

private:
    void upload(const std::vector<float>& s_vertices, const std::vector<uint32_t>& s_indices);
    void release(); // not const because it releases the objects linked to 
					// m_vao, m_vbo and m_ebo, not inherently modifying them
					// but modifying what they are linked to
    
    unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;
    GLsizei n_indices = 0;
};