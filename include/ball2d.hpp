#ifndef BALL2D_HPP
#define BALL2D_HPP

#include <string>
#include <algorithm>
#include <random>

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

//class Billiard2D;

struct BallState {
    glm::vec3 displacement;
    glm::vec3 speed;
    glm::vec3 acceleration;
};

class Ball2D {
    float cx, cy;
    float cz;
    Shader *shader_ptr;
    
    glm::mat4 transform;

    int   *indices;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int texture;
    void triangulate();

    BallState old_state;
    int idx;

    //Billiard2D* game_ptr = NULL; //so that we can access the geometries of the game easily
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

    Ball2D(int idx, float cx, float cy, float r, std::string texture_path);
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

    BallState getState() {
        return BallState{displacement, speed, acceleration};
    }

    void setState(const BallState &state) {
        displacement = state.displacement;
        speed        = state.speed;
        acceleration = state.acceleration;
        cx1          = displacement.x;
        cy1          = displacement.y;
    }

    void printState() {
        std::cout << "DIS: " << displacement.x << " " << displacement.y << " " << displacement.z << std::endl;
        std::cout << "VEL: " << speed.x << " " << speed.y << " " << speed.z << std::endl;
        std::cout << "ACC: " << acceleration.x << " " << acceleration.y << " " << acceleration.z << std::endl;
    }

    void updateAcceleration() {
        if (glm::length(speed) > 1e-5) {
            acceleration = -(sliding_fraction * mass * 0.4f) * glm::normalize(speed);
        } else {
            speed = glm::vec3(0.f);
            acceleration = glm::vec3(0.f);
        }
    }

    void backupState() {
        old_state.displacement = displacement;
        old_state.speed        = speed;
        old_state.acceleration = acceleration;
    }

    void recoverOldState() {
        displacement           = old_state.displacement;
        speed                  = old_state.speed;
        acceleration           = old_state.acceleration;
        cx1                    = displacement.x;
        cy1                    = displacement.y;
    }

    void handleCollision(Ball2D &ball);

    bool isMoving() {
        return glm::length(speed) > 1e-5;
    }

    //void setGamePtr(Billiard2D* game_ptr);
};

#endif