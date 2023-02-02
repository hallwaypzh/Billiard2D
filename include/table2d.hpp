#ifndef TABLE2D_HPP
#define TABLE2D_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <iostream>


#include "shader_m.hpp"

class Table2D {

    unsigned char *texture_data;
    float *vertices;
    int *indices;
    Shader *shader_ptr;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int texture;

    glm::mat4 view_project_mat = glm::mat4(1.f);


public:
    Table2D();
    ~Table2D() {
        delete vertices;
        delete indices;
        delete shader_ptr;

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteTextures(1, &texture);
    }

    void render();

    void setViewProjectMatrix(glm::mat4 mat) {
        view_project_mat = mat;
    }
};

#endif