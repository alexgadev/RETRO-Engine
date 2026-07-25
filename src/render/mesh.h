#pragma once

#include <../../include/glad/glad.h> // holds all OpenGL type declarations

struct Vertex 
{
    glm::vec3 Position;
    
    //glm::vec3 Normal;

    glm::vec2 TexCoords;
};

class Mesh 
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    unsigned int texture;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, unsigned int texture)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->texture = texture;

        setupMesh();
    }

    void draw(Shader &shader)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    }

private:
    unsigned int VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
	    glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

	    glBindVertexArray(VAO);

	    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	    // vertex positions
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	    glEnableVertexAttribArray(0);
        // vertex normals
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	    glEnableVertexAttribArray(1);
        // texture coord attribute
	    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	    glEnableVertexAttribArray(2);
    }
};