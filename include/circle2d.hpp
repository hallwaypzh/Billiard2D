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

enum BallStatus {
    STATIC,
    ROLLING,
    FALLING,
};

class Circle2D {
    float cx, cy;
    float cz;

    
    int ntriangles;
    Shader *shader_ptr;
    
    glm::mat4 transform;

    int   *indices;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    void triangulate();
public:
    float *vertices;
    float r;
    float cx1, cy1;
    BallStatus status;
    Circle2D *pocket_ptr;
    glm::mat4 view_project_mat;
    glm::vec3 speed;
    glm::vec3 acceleration;
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

    // void set_z(float z) {
    //     int i = 0;
    //     for (i=0; i< this->ntriangles; i++) {
    //         vertices[3*i+2] = z;
    //     }
    //     vertices[3*i+2] = z;

    // }

    bool isFalling(const Circle2D &pocket);
    

    void updateTransform() {
        this->transform[3][0] = cx1 - cx;
        this->transform[3][1] = cy1 - cy;
        this->transform[3][2] = cz;
    }

    void set_z(float z) {
        this->cz = z;
        this->updateTransform();
    }
};

#endif