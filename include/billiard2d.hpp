#ifndef GAME_HPP
#define GAME_HPP

#include <glm/glm.hpp>

#include "camera.hpp"
#include "circle2d.hpp"


class Billiard2D {

    Camera *camera_ptr;
    Circle2D *circle_ptr;

public:
    Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far);
    ~Billiard2D() {
        delete camera_ptr;
        delete circle_ptr;
    }
    void processInput(float delta_time);
    void updateScene(float delta_time);
    void render();
    void updateAspectRatio(float aspect_ratio) {
        this->camera_ptr->update_aspect_ratio(aspect_ratio);
        this->circle_ptr->setViewProjectMatrix(
            camera_ptr->project * camera_ptr->view
        );
    }
};

#endif