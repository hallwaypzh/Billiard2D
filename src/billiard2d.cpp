#include "billiard2d.hpp"


Billiard2D::Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far) {
    camera_ptr = new Camera(fovy, aspect_ratio, z_near, z_far);
    circle_ptr = new Ball2D(0.1f, 0.2f, 0.015f, 360, "shaders/circle.vs", "shaders/circle.fs");
    circle_ptr->sliding_fraction = 0.2;
    circle_ptr->set_z(-0.049f);
    circle_ptr->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
    circle_ptr->speed = glm::vec3(0.f, 0.0f, 0.f);
    table_ptr = new Table2D(camera_ptr->project * camera_ptr->view);
    cue_ptr = new Cue2D(camera_ptr->project * camera_ptr->view);
}

void Billiard2D::processInput(float delta_time) {
    
}

void Billiard2D::updateScene(float delta_time) {
    this->cue_ptr->update(delta_time);
    int i = 0;
    //std::cout << "***************************\n";    
    if (this->circle_ptr != NULL) {
        if (cue_ptr->state == FIRING) {
            glm::vec4 tip = cue_ptr->translation_mat1 * cue_ptr->translation_mat0 * cue_ptr->rotation_mat * glm::vec4(0.f, 0.f, -0.05f, 1.f);
            glm::vec3 v = glm::vec3(tip)-glm::vec3(circle_ptr->cx1, circle_ptr->cy1, -0.05f);
            float diff = circle_ptr->r - glm::length(v);
            if (diff > 0) {
                // ball collide with cue
                glm::vec3 v1 = diff * glm::normalize(v);
                cue_ptr->translation_mat0 = glm::translate(cue_ptr->translation_mat0, v1);
                circle_ptr->speed = cue_ptr->speed * cue_ptr->mass / circle_ptr->mass;
                cue_ptr->speed = glm::vec3(0.f);
                cue_ptr->acceleration = glm::vec3(0.f);
                cue_ptr->state = STOPPED;
                std::cout << "CUE STOPPED MOVING! BALL START MOVING!\n";
            }
        }
        this->circle_ptr->update(delta_time);
        if (this->circle_ptr->status != FALLING) {
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
            for (auto pocket : table_ptr->pockets) {
                if (this->circle_ptr->isFalling(*pocket)) {
                    this->circle_ptr->status = FALLING;
                    this->circle_ptr->pocket_ptr = pocket;
                    delete this->circle_ptr;
                    this->circle_ptr = NULL;
                    break;
                }
            }
        } else {
            if (!this->circle_ptr->isFalling(*this->circle_ptr->pocket_ptr)) {
                this->circle_ptr->speed = glm::vec3(0.f);
                glm::vec2 p0(this->circle_ptr->cx1, this->circle_ptr->cy1);
                glm::vec2 p1(this->circle_ptr->pocket_ptr->cx1, this->circle_ptr->pocket_ptr->cy1);
                glm::vec2 p = glm::normalize(p0 - p1) * (this->circle_ptr->pocket_ptr->r - this->circle_ptr->r) + p1;
                this->circle_ptr->cx1 = p.x;
                this->circle_ptr->cy1 = p.y;
                this->circle_ptr->updateTransform();
            }
        }
        if (cue_ptr->state==STOPPED && noBallsMoving()) {
            std::cout <<  "Ball stopped moving\n";
            phase = ADJUST_ANGLE;
            player_active = true;
            cue_ptr->state = ADJUSTING;
            circle_ptr->speed = glm::vec3(0.f);
        }
    }
}

void Billiard2D::render() {
    cue_ptr->render();
    this->table_ptr->render();
    if (this->circle_ptr != NULL) {
        this->circle_ptr->render();
    }
}

void Billiard2D::updateCueAngle(glm::vec3 v) {
    if (player_active && circle_ptr != NULL) {
        if (phase == ADJUST_ANGLE) {
            cue_ptr->translation_mat0 = glm::mat4(1.f);
            glm::vec3 v0(0.f, 1.f, -0.05f);
            old_cursor_coord = glm::vec2(v.x, v.y);
            glm::vec3 v1 = glm::vec3(circle_ptr->cx1, circle_ptr->cy1, -0.05f) - v;
            float theta = glm::acos(glm::dot(v0, v1) / (glm::length(v0) * glm::length(v1)));
            //std::cout << theta << std::endl;
            if (v1.x > 0.f) {
                theta = 2 * M_PI - theta;
            }
            glm::vec3 axis = glm::vec3(0.f, 0.f, 1.f);
            cue_ptr->rotation_mat = glm::rotate(glm::mat4(1.f), theta, axis);
            cue_ptr->translation_mat1 = glm::translate(glm::translate(glm::mat4(1.f), glm::vec3(circle_ptr->cx1, circle_ptr->cy1, 0.f)), -0.03f * glm::vec3(glm::normalize(glm::vec2(v1.x, v1.y)), 0.f));
        } else if (phase == ADJUST_STRENGTH) {
            float strength = 1*glm::length(glm::vec2(v.x, v.y) - old_cursor_coord);
            strength = std::min(0.3f, strength);
            glm::vec4 v1 = cue_ptr->translation_mat1 * cue_ptr->rotation_mat * glm::vec4(0.f, 0.f, -0.05f, 1.f);
            glm::vec3 v2 = glm::normalize(glm::vec3(circle_ptr->cx1, circle_ptr->cy1, -0.05f) - glm::vec3(v1.x, v1.y, v1.z));
            cue_ptr->translation_mat0 = glm::translate(glm::mat4(1.f), (-strength)*v2);
        }
    }
}


void Billiard2D::updatePlayerPhase(int button, int action) {
    if (circle_ptr == NULL) {
        return ;
    }
    if (phase != RELEASE_CUE) {
        if (phase == ADJUST_ANGLE) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                phase = ADJUST_STRENGTH;
                mouse_button_pressed[GLFW_MOUSE_BUTTON_LEFT] = true;
            }

        } else if (phase == ADJUST_STRENGTH) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS) {
                    mouse_button_pressed[GLFW_MOUSE_BUTTON_LEFT] = true;
                } else {
                    if (!mouse_button_pressed[GLFW_MOUSE_BUTTON_RIGHT]) {
                        player_active = false;
                        phase = RELEASE_CUE;
                        glm::vec4 tip = cue_ptr->translation_mat1 * cue_ptr->rotation_mat * glm::vec4(0.f, 0.f, -0.05f, 1.f);
                        glm::vec3 d = glm::vec3(circle_ptr->cx1, circle_ptr->cy1, -0.05f) - glm::vec3(tip);
                        cue_ptr->state = FIRING;
                        cue_ptr->acceleration = 0.5f * d;
                        cue_ptr->speed = glm::vec3(0.f);
                        cue_ptr->displacement = glm::vec3(0.f);
                    }
                    mouse_button_pressed[GLFW_MOUSE_BUTTON_LEFT] = false;
                }
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (action == GLFW_RELEASE) {
                    phase = ADJUST_ANGLE;
                    mouse_button_pressed[GLFW_MOUSE_BUTTON_RIGHT] = false;
                } else {
                    mouse_button_pressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
                }
            }
        }
    }
}

bool Billiard2D::noBallsMoving() {
    if (circle_ptr) {
        return glm::length(circle_ptr->speed) < 1e-5;
    }
    return true;
}