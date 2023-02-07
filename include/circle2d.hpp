#ifndef CIRCLE2D_H
#define CIRCLE2D_H

#include <string>
#include <algorithm>

#include <shader_m.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "line2d.hpp"

class Circle2D {
    float cx, cy;
    float cx1, cy1;
    float r;
    int ntriangles;
    Shader *shader_ptr;
    float *vertices;
    
    glm::mat4 transform;
    

    int   *indices;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    void triangulate();



public:
    glm::mat4 view_project_mat;
    glm::vec3 speed;
    Circle2D(float cx, float cy, float r, int ntriangles, std::string vertex_shader_path, std::string fragment_shader_path);
    ~Circle2D() {
        delete shader_ptr;
        delete vertices;
        delete indices;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void render();
    void update(float deltaTime);

    void setViewProjectMatrix(glm::mat4 mat) {
        this->view_project_mat = mat;
    }

    bool collide(const LineSegment2D &segment);

    Circle2D(float cx, float cy, float cz, float r, int ntriangles, std::string vertex_shader_path, std::string fragment_shader_path);

    void set_z(float z) {
        int i = 0;
        for (int i=0; i< this->ntriangles; i++) {
            vertices[3*i+2] = z;
        }
    }
    
};

#endif