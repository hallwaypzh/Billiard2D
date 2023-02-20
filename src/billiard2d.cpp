#include "billiard2d.hpp"


Billiard2D::Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far) {
    camera_ptr = new Camera(fovy, aspect_ratio, z_near, z_far);
    circle_ptr = new Ball2D(0.1f, 0.2f, 0.015f, 360, "res/textures/ball0.png");
    circle_ptr->sliding_fraction = 0.2;
    circle_ptr->set_z(-0.049f);
    circle_ptr->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
    //circle_ptr->setGamePtr(this);

    circle_ptr1 = new Ball2D(0.1f, -0.f, 0.015f, 360, "res/textures/ball1.png");
    circle_ptr1->sliding_fraction = 0.2;
    circle_ptr1->set_z(-0.049f);
    circle_ptr1->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
    //circle_ptr1->setGamePtr(this);


    // set up the balls
    glm::vec3 d1(sqrtf(3)/2.f, 0.5f, 0.f);
    glm::vec3 d2(sqrtf(3)/2.f, -0.5f, 0.f);
    for (int i=0; i<16; i++) {
        balls[i] = NULL;
    }
    int idx = 1;
    float eps = 2e-3;
    glm::vec3 p0(0.f);
    float radius = 0.015f;
    float d = 2 * radius + eps;
    for (int i=0; i<5; i++) {
        for (int j=0; j<=i; j++) {
            glm::vec3 p = ((j * 1.f) * d1 + ((i-j) * 1.f) * d2) * d;
            std::cout <<"Ball " << idx << " " << p.x << " " << p.y << std::endl;
            char texture_path[50];
            sprintf(texture_path, "res/textures/ball%d.png", idx);
            balls[idx] = new Ball2D(p.x, p.y, radius, 360, texture_path);
            balls[idx]->set_z(-0.049f+1e-5*idx); // prevent z fighting
            balls[idx]->setViewProjectMatrix(
                camera_ptr->project * camera_ptr->view
            );
            idx++;
        }
    }


    table_ptr = new Table2D(camera_ptr->project * camera_ptr->view);
    cue_ptr = new Cue2D(camera_ptr->project * camera_ptr->view);
}

void Billiard2D::processInput(float delta_time) {
    
}

void Billiard2D::updateScene(float delta_time) {
    this->cue_ptr->update(delta_time);
    int i = 0;
    //std::cout << "***************************\n";    
    if (cue_ptr->state == FIRING && this->circle_ptr != NULL) {
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
            phase = BALLS_MOVING;
            std::cout << "CUE STOPPED MOVING! BALL START MOVING!\n";
        }
    }

    if (phase == BALLS_MOVING) {
        // collect old states
        if (circle_ptr) {
            circle_ptr->update(delta_time);
        }
        if (circle_ptr1) {
            circle_ptr1->update(delta_time);
        }
        //std::cout << glm::length(circle_ptr->displacement-circle_ptr1->displacement) << std::endl;

        // the collision handling doesn't make sense by far, but ...
        // check if falling
        if (circle_ptr) {
            for (auto pocket : table_ptr->pockets) {
                if (circle_ptr->isFalling(*pocket)) {
                    circle_ptr->status = FALLING;
                    circle_ptr->pocket_ptr = pocket;
                    delete circle_ptr;
                    circle_ptr = NULL;
                    break;
                }
            }
        }

        if (circle_ptr1) {
            for (auto pocket : table_ptr->pockets) {
                if (circle_ptr1->isFalling(*pocket)) {
                    circle_ptr1->status = FALLING;
                    circle_ptr1->pocket_ptr = pocket;
                    delete circle_ptr1;
                    circle_ptr1 = NULL;
                    break;
                }
            }
        }

        // check if hit the line 
        if (circle_ptr) {
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
        }

        if (circle_ptr1) {
            for (auto line : table_ptr->lines) {
                //print(this->circle_ptr->collide(table_ptr->lines[]))
                if (circle_ptr1->collide(line)) {
                    if (line.layout == LEFT || line.layout == RIGHT) {
                        this->circle_ptr1->speed.x = -this->circle_ptr1->speed.x; 
                    } else {
                        this->circle_ptr1->speed.y = -this->circle_ptr1->speed.y;
                    }
                    break;
                }
            }
        }

        if (circle_ptr && circle_ptr1 && circle_ptr->collide(*circle_ptr1)) {
            //using namespace glm;
            std::cout << "**************\n";
            std::cout << glm::length(circle_ptr1->displacement-circle_ptr->displacement) << std::endl;
            std::cout << "Collide!\n";
            circle_ptr1->recoverOldState();
            circle_ptr->recoverOldState();
            //std::cout << glm::length(circle_ptr1->displacement-circle_ptr->displacement) << std::endl;
            float t = circle_ptr->getCollideTime(*circle_ptr1, delta_time, 1e-5);
            circle_ptr->update(t);
            circle_ptr1->update(t);
            std::cout << glm::length(circle_ptr1->displacement-circle_ptr->displacement) << std::endl;

            glm::vec3 speed0 = circle_ptr1->speed;
            glm::vec3 relative_speed = circle_ptr->speed - speed0;
            glm::vec3 d = glm::normalize(circle_ptr1->displacement - circle_ptr->displacement);
            glm::vec3 relative_speed_para = glm::dot(relative_speed, d) * d;
            glm::vec3 relative_speed_perp = relative_speed - relative_speed_para;
            circle_ptr->speed = relative_speed_perp + speed0;
            circle_ptr1->speed = relative_speed_para + speed0;

            circle_ptr1->updateAcceleration();
            circle_ptr->updateAcceleration();
            // circle_ptr->speed = circle_ptr1->speed;
            // circle_ptr->acceleration = circle_ptr1->acceleration;
            // circle_ptr1->speed = middle_states.speed;
            // circle_ptr1->acceleration = middle_states.acceleration;
        }

        if (cue_ptr->state==STOPPED && noBallsMoving()) {
            std::cout <<  "Ball stopped moving\n";
            phase = ADJUST_ANGLE;
            player_active = true;
            cue_ptr->state = ADJUSTING;
            circle_ptr->speed = glm::vec3(0.f);
            circle_ptr->acceleration = glm::vec3(0.f);
        }
    }
}

void Billiard2D::render() {
    cue_ptr->render();
    this->table_ptr->render();
    if (this->circle_ptr != NULL) {
        this->circle_ptr->render();
    }
    // if (this->circle_ptr1 != NULL) {
    //     this->circle_ptr1->render();
    // }

    for (int i=1; i<16; i++) {
        if (balls[i] != NULL) {
            balls[i]->render();
        }
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
    bool moving1 = false;
    bool moving2 = false;

    if (circle_ptr) {
        moving1 = glm::length(circle_ptr->speed) > 1e-5;
        //return glm::length(circle_ptr->speed) < 1e-5;
    }
    if (circle_ptr1) {
        moving2 = glm::length(circle_ptr1->speed) > 1e-5;
    }
    return !(moving1 || moving2);
}