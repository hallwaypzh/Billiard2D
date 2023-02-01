#include "billiard2d.hpp"


Billiard2D::Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far) {
    camera_ptr = new Camera(fovy, aspect_ratio, z_near, z_far);
    circle_ptr = new Circle2D(0.f, 0.f, 0.1f, 360, "shaders/circle.vs", "shaders/circle.fs");
    circle_ptr->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
}

void Billiard2D::processInput(float delta_time) {
    
}

void Billiard2D::updateScene(float delta_time) {
    this->circle_ptr->update(delta_time);
}

void Billiard2D::render() {
    this->circle_ptr->render();
}