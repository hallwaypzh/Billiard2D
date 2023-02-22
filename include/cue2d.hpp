#ifndef CUE2D_HPP
#define CUE2D_HPP

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>
#include <algorithm>

#include "shader_m.hpp"

enum CueState {
    ADJUSTING,
    FIRING,
    STOPPED
};

class Cue2D {
    unsigned char *texture_data;
    float *vertices;
    int *indices;
    Shader *shader_ptr;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int texture;

    unsigned int FADE_FRAMES = 10;

    glm::mat4 view_project_mat = glm::mat4(1.f);
public:
    unsigned int fade_frame_cnt = 0;
    float alpha = 1.f;
    glm::vec3 speed = glm::vec3(0.f);
    glm::vec3 acceleration = glm::vec3(0.f);
    glm::vec3 displacement = glm::vec3(0.f);
    glm::mat4 translation_mat0 = glm::mat4(1.f);
    glm::mat4 translation_mat1 = glm::mat4(1.f);
    glm::mat4 rotation_mat = glm::mat4(1.f);
    float     mass         = 2.f;
    CueState state = ADJUSTING;
    Cue2D(const glm::mat4 &view_project_mat);
    ~Cue2D() {
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

    void update(float delta_time);
};

#endif