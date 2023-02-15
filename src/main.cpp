#include "glad/glad.h"
#include <GLFW/glfw3.h>


#include <iostream>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>


#include "billiard2d.hpp"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_call_back(GLFWwindow* window, double xpos, double ypos);
void cursor_button_call_back(GLFWwindow*, int, int, int);
void processInput(GLFWwindow *window);

void generate_circle(float cx, float cy, float radius, int ntriangles, float *vertices, int *indices);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int current_screen_width, current_screen_height;

Billiard2D *billiard_game_ptr;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Billiards", NULL, NULL);

    current_screen_height = SCR_HEIGHT;
    current_screen_width = SCR_WIDTH;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //Circle2D *circle_ptr = new Circle2D(0.f, 0.f, 0.1f, 360, "shaders/circle.vs", "shaders/circle.fs");
    billiard_game_ptr = new Billiard2D(glm::radians(22.5f), SCR_WIDTH * 1.f / SCR_HEIGHT, 2.9f, 100.f);
    glfwSetCursorPosCallback(window, cursor_position_call_back);
    glfwSetMouseButtonCallback(window, cursor_button_call_back);

    // render loop
    // -----------
    float last_time = (float) glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        billiard_game_ptr->updateScene((float)glfwGetTime() - last_time);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // circle.render();
        billiard_game_ptr->render();
        last_time = (float)glfwGetTime();
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    delete billiard_game_ptr;
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.

    // float ar1 = (width * 1.f) / height;
    // float ar0 = (SCR_WIDTH * 1.f) / SCR_HEIGHT;
    // if (ar1 > ar0) {
    //     float width1 = ar0 * height;
    //     float dx = (width - width1) / 2.;
    //     glViewport((GLint)dx, 0, (GLint)(dx+width1), height);
    //     std::cout << (width1 / height)  << " " << ar1 << " " << dx << " " << ar0 << std::endl;
    // } else {
    //     float height1 = width * 1.f / ar0;
    //     float dy = (height - height1) / 2.;
    //     glViewport(0, (GLint)dy, width, (GLint)(dy+height1));
    //     std::cout << (width / height1) << " " << ar0 << std::endl;
    // }
    glViewport(0, 0, width, height);
    // current_screen_height = height;
    // current_screen_width  = width;
    billiard_game_ptr->updateAspectRatio(width * 1.f / height);
}

void cursor_position_call_back(GLFWwindow* window, double xpos, double ypos) {
    int window_height, window_width;
    glfwGetWindowSize(window, &window_width, &window_height);
    glm::mat3x3 mat(
        2.f/window_width, 0.f, 0.f,
        0.f, -2.f/window_height, 0.f,
        -1.f, 1.f, 1.f
    );

    glm::vec3 v = mat * glm::vec3(xpos, ypos, 1.);   
    glm::mat4 mat_vp = billiard_game_ptr->camera_ptr->project * billiard_game_ptr->camera_ptr->view;
    glm::vec4 v1 = (mat_vp * glm::vec4 (0.f, 0.f, -0.05f, 1.f));
    v.z = v1.z / v1.w;
    glm::vec4 v2 = glm::inverse(mat_vp) * glm::vec4(v, 1.f);
    billiard_game_ptr->updateCueAngle(glm::vec3(v2.x/v2.w, v2.y/v2.w, v2.z/v2.w));
}

void cursor_button_call_back(GLFWwindow* window, int button, int action, int mods) {
    billiard_game_ptr->updatePlayerPhase(button, action);
}