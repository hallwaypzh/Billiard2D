#ifndef BALL2D_HPP
#define BALL2D_HPP

#include <string>
#include <algorithm>

#include <shader_m.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "line2d.hpp"
#include "circle2d.hpp"

// enum BallStatus {
//     STATIC,
//     ROLLING,
//     FALLING,
// };

class Ball2D {
    float cx, cy;
    float cz;
    int ntriangles;
    Shader *shader_ptr;
    
    glm::mat4 transform;

    int   *indices;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int texture;
    void triangulate();
public:
    float *vertices;
    float r;
    float mass;
    float cx1, cy1;
    float sliding_fraction;
    BallStatus status;
    Circle2D *pocket_ptr;
    glm::mat4 view_project_mat;
    glm::vec3 speed;
    glm::vec3 acceleration;
    glm::vec3 displacement;

    Ball2D(float cx, float cy, float r, int ntriangles, std::string vertex_shader_path, std::string fragment_shader_path);
    ~Ball2D() {
        delete shader_ptr;
        delete vertices;
        delete indices;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &texture);
    }
    void render();
    void update(float deltaTime);

    void setViewProjectMatrix(glm::mat4 mat) {
        this->view_project_mat = mat;
    }

    bool collide(const LineSegment2D &segment);
    bool collide(const Ball2D &ball);

    float getCollideTime(const Ball2D &ball, float deltaTime, float absoluteTolerance);
    bool isFalling(const Circle2D &pocket);
    

    void updateTransform() {
        this->transform[3][0] = cx1;
        this->transform[3][1] = cy1;
        this->transform[3][2] = cz;
    }

    void set_z(float z) {
        this->cz = z;
        this->updateTransform();
    }
};

#endif