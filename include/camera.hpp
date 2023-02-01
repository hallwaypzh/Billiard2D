#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

class Camera {
    float fovy;
    float z_near;
    float z_far;

public:
    glm::mat4 project;
    glm::mat4 view;
    Camera(float fovy, float aspect_ratio, float z_near, float z_far) {
        view = glm::lookAt(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        project = glm::perspective(fovy, aspect_ratio, z_near, z_far);
        this->fovy = fovy;
        this->z_far = z_far;
        this->z_near = z_near;
    }

    void update_aspect_ratio(float aspect_ratio) {
        project = glm::perspective(fovy, aspect_ratio, z_near, z_far);
    }
};

#endif 