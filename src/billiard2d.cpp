#include "billiard2d.hpp"


Billiard2D::Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far) {
    
    camera_ptr = new Camera(fovy, aspect_ratio, z_near, z_far);

    //circle_ptr->setGamePtr(this);
    float radius = 0.02f;

    // set up the balls
    glm::vec3 d0(0.2f, 0.f, 0.f);
    glm::vec3 d1(sqrtf(3)/2.f, 0.5f, 0.f);
    glm::vec3 d2(sqrtf(3)/2.f, -0.5f, 0.f);
    for (int i=1; i<16; i++) {
        balls[i] = NULL;
    }
    int idx = 1;
    float eps = 1e-6f;

    glm::vec3 p0(0.f);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(2, 5);

    balls[0] = new Ball2D(0, -0.2f, 0.2f, radius, "res/textures/ball0.png");
    balls[0]->sliding_fraction = 0.3;
    balls[0]->set_z(-0.049f);
    balls[0]->setViewProjectMatrix(
        camera_ptr->project * camera_ptr->view
    );
    //std::cout << balls[0].cx << " " << balls[0].cy << std::endl;
    
    
    float d = 2 * radius + eps;
    // only 6 colored balls for the purpose of debugging
    for (int i=0; i<5; i++) {
        for (int j=0; j<=i; j++) {
            glm::vec3 p = d0 + ((j * 1.f) * d1 + ((i-j) * 1.f) * d2) * d;
            std::cout <<"Ball " << idx << " " << p.x << " " << p.y << std::endl;
            char texture_path[50];
            sprintf(texture_path, "res/textures/ball%d.png", idx);
            balls[idx] = new Ball2D(idx, p.x, p.y, radius, texture_path);
            balls[idx]->sliding_fraction = 0.3;
            balls[idx]->set_z(-0.049f+1e-5*idx); // prevent z fighting
            balls[idx]->setViewProjectMatrix(
                camera_ptr->project * camera_ptr->view
            );
            idx++;
        }
    }

    // for (int i=1; i<16; i++) {
    //     for (int j=i+1; j<16; j++) {
    //         std::cout << i << " " << j << " " << balls[i]->collide(*(balls[j])) << std::endl;
    //     }
    // }
    table_ptr = new Table2D(camera_ptr->project * camera_ptr->view);
    cue_ptr = new Cue2D(camera_ptr->project * camera_ptr->view);
}

void Billiard2D::processInput(float delta_time) {
    
}

void Billiard2D::updateScene(float delta_time) {
    //std::cout << delta_time << std::endl;
    this->cue_ptr->update(delta_time);
    int i = 0;
    //std::cout << "***************************\n";    
    if (cue_ptr->state == FIRING) { // it's guaranteed that when the cue is fired, the white ball is still on the table
        glm::vec4 tip = cue_ptr->translation_mat1 * cue_ptr->translation_mat0 * cue_ptr->rotation_mat * glm::vec4(0.f, 0.f, -0.05f, 1.f);
        glm::vec3 v = glm::vec3(tip)-glm::vec3(balls[0]->cx1, balls[0]->cy1, -0.05f);
        float diff = balls[0]->r - glm::length(v);
        if (diff > 0) {
            // ball collide with cue
            glm::vec3 v1 = diff * glm::normalize(v);
            cue_ptr->translation_mat0 = glm::translate(cue_ptr->translation_mat0, v1);
            balls[0]->speed = cue_ptr->speed * cue_ptr->mass / balls[0]->mass;
            std::cout << balls[0]->speed.x << " " << balls[0]->speed.y << std::endl;
            cue_ptr->speed = glm::vec3(0.f);
            cue_ptr->acceleration = glm::vec3(0.f);
            cue_ptr->state = STOPPED;
            phase = BALLS_MOVING;
            std::cout << "CUE STOPPED MOVING! BALL START MOVING!\n";
        }
    }

    if (phase == BALLS_MOVING) {
        // collect old states
        for (auto ball:balls) {
            if (ball) {
                ball->update(delta_time);
            }
        }
        //std::cout << glm::length(circle_ptr->displacement-circle_ptr1->displacement) << std::endl;

        // the collision handling doesn't make sense by far, but ...
        // check if falling
        for (auto &ball:balls) {
            if (ball && ball->isMoving()) {
                for (auto pocket : table_ptr->pockets) {
                    if (ball->isFalling(*pocket)) {
                        ball->status = FALLING;
                        ball->pocket_ptr = pocket;
                        if (ball == balls[0]) {
                            phase = ADJUST_ANGLE;
                            player_active = true;
                        }
                        delete ball;
                        ball = NULL;
                        break;
                    }
                }
            }
        }

        // check if hit the line 
        for (auto ball:balls) {
            if (ball && ball->isMoving()) {
                for (auto line:table_ptr->lines) {
                    if (ball->collide(line)) {
                        if (line.layout == LEFT || line.layout == RIGHT) {
                            ball->speed.x = -ball->speed.x; 
                        } else {
                            ball->speed.y = -ball->speed.y;
                        }
                        ball->backupState();
                        break;
                    }
                }
            }
        }

        float min_t = delta_time+100.f;
        int idx1 = -1, idx2 = -1; //index for potential collision candidates

        for (int i=0; i<16; i++) {
            if (balls[i]) {
                for (int j=i+1; j<16; j++) {
                    if (balls[j]) {
                        if (balls[i]->collide(*(balls[j]))) {
                            std::cout << "COLLSION\n";
                            float t = balls[i]->getCollideTime(*(balls[j]), delta_time, 1e-5);
                            // std::cout << "COLLIDE!" << i << " " << j << " " << t << std::endl;
                            if (t < min_t) {
                                min_t = t;
                                idx1 = i;
                                idx2 = j;
                            }
                        }
                    }
                }
            }
        }
        //std::cout << "SELETED BALLS" << idx1 << " " << idx2 << " " << "COLLISION TIME " << min_t << std::endl;

        if (min_t < delta_time) {
            for (auto ball:balls) {
                if (ball) {
                    ball->recoverOldState();
                    ball->update(min_t);
                }
            }
            balls[idx1]->handleCollision(*(balls[idx2]));
            if (delta_time-min_t > 1e-3) {
                updateScene(delta_time-min_t);
            }
        }


        if (cue_ptr->state==STOPPED && noBallsMoving()) {
            std::cout <<  "Ball stopped moving\n";
            phase = ADJUST_ANGLE;
            player_active = true;
            cue_ptr->state = ADJUSTING;
            // if (balls[0]) {
            //     balls[0]->speed = glm::vec3(0.f);
            //     balls[0]->acceleration = glm::vec3(0.f);
            // }
        }
    }
}

void Billiard2D::render() {
    cue_ptr->render();
    table_ptr->render();
    for (auto ball:balls) {
        if (ball) {
            ball->render();
        }
    }
}

void Billiard2D::updateCueAngle(glm::vec3 v) {
    if (player_active && balls[0] != NULL) {
        if (phase == ADJUST_ANGLE) {
            cue_ptr->translation_mat0 = glm::mat4(1.f);
            glm::vec3 v0(0.f, 1.f, -0.05f);
            old_cursor_coord = glm::vec2(v.x, v.y);
            glm::vec3 v1 = glm::vec3(balls[0]->cx1, balls[0]->cy1, -0.05f) - v;
            float theta = glm::acos(glm::dot(v0, v1) / (glm::length(v0) * glm::length(v1)));
            //std::cout << theta << std::endl;
            if (v1.x > 0.f) {
                theta = 2 * M_PI - theta;
            }
            glm::vec3 axis = glm::vec3(0.f, 0.f, 1.f);
            cue_ptr->rotation_mat = glm::rotate(glm::mat4(1.f), theta, axis);
            cue_ptr->translation_mat1 = glm::translate(glm::translate(glm::mat4(1.f), glm::vec3(balls[0]->cx1, balls[0]->cy1, 0.f)), -0.03f * glm::vec3(glm::normalize(glm::vec2(v1.x, v1.y)), 0.f));
        } else if (phase == ADJUST_STRENGTH) {
            float strength = 1*glm::length(glm::vec2(v.x, v.y) - old_cursor_coord);
            strength = std::min(0.3f, strength);
            glm::vec4 v1 = cue_ptr->translation_mat1 * cue_ptr->rotation_mat * glm::vec4(0.f, 0.f, -0.05f, 1.f);
            glm::vec3 v2 = glm::normalize(glm::vec3(balls[0]->cx1, balls[0]->cy1, -0.05f) - glm::vec3(v1.x, v1.y, v1.z));
            cue_ptr->translation_mat0 = glm::translate(glm::mat4(1.f), (-strength)*v2);
        }
    }
}


void Billiard2D::updatePlayerPhase(int button, int action) {
    if (balls[0] == NULL) {
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
                        glm::vec3 d = glm::vec3(balls[0]->cx1, balls[0]->cy1, -0.05f) - glm::vec3(tip);
                        cue_ptr->state = FIRING;
                        cue_ptr->acceleration = 2.5f * d;
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
    for (auto ball:balls) {
        if (ball) {
            if (glm::length(ball->speed)>1e-5) {
                return false;
            }
        }
    }
    return true;
}