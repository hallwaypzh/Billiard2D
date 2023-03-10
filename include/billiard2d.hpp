#ifndef GAME_HPP
#define GAME_HPP

#include <cstdio>
#include <random>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "circle2d.hpp"
#include "table2d.hpp"
#include "cue2d.hpp"
#include "ball2d.hpp"

enum PlayerPhase {
    ADJUST_ANGLE,
    ADJUST_STRENGTH,
    RELEASE_CUE,
    BALLS_MOVING
};


class Billiard2D {
    //Circle2D *circle_ptr;
    // Ball2D      *circle_ptr;
    Table2D *table_ptr;
    Cue2D *cue_ptr;
    bool player_active = true;
    PlayerPhase phase = ADJUST_ANGLE;
    glm::vec2 old_cursor_coord;

    bool mouse_button_pressed[2] = {false, false};

    Ball2D *balls[16];
 
public:
    Camera *camera_ptr;
    Billiard2D(float fovy, float aspect_ratio, float z_near, float z_far);
    ~Billiard2D() {
        delete camera_ptr;
        for (int i=0; i<16; i++) {
            if (balls[i] != NULL) {
                delete balls[i];
            }
        }
        delete table_ptr;
    }
    void processInput(float delta_time);
    void updateScene(float delta_time);
    void render();
    void updateAspectRatio(float aspect_ratio) {
        this->camera_ptr->update_aspect_ratio(aspect_ratio);
        for (auto ball:balls) {
            if (ball) {
                ball->setViewProjectMatrix(
                    camera_ptr->project * camera_ptr->view
                );
            }
        }
    }

    void updateCueAngle(glm::vec3 v);

    void updatePlayerPhase(int button, int action);

    bool noBallsMoving();
};

#endif