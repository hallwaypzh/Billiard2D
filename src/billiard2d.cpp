#include "billiard2d.hpp"


Billiard2D::Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far) {
    camera_ptr = new Camera(fovy, aspect_ratio, z_near, z_far);
    circle_ptr = new Circle2D(0.f, 0.f, 0.02f, 360, "shaders/circle.vs", "shaders/circle.fs");
    circle_ptr->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
    table_ptr = new Table2D();
    table_ptr->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
}

void Billiard2D::processInput(float delta_time) {
    
}

void Billiard2D::updateScene(float delta_time) {
    this->circle_ptr->update(delta_time);
    
    int i = 0;
    //std::cout << "***************************\n";
    for (auto line : table_ptr->lines) {
        //print(this->circle_ptr->collide(table_ptr->lines[]))
        if (this->circle_ptr->collide(line)) {
            if (line.layout == LEFT || line.layout == RIGHT) {
                this->circle_ptr->speed.x = -this->circle_ptr->speed.x; 
            } else {
                this->circle_ptr->speed.y = -this->circle_ptr->speed.y;
            }
            break;
        }
        
    }
    //std::cout << "***************************\n";
    
}

void Billiard2D::render() {
    this->circle_ptr->render();
    this->table_ptr->render();
}
