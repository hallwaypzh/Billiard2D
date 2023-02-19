#include "ball2d.hpp"
#include "stb_image.h"

Ball2D::Ball2D(float cx, float cy, float r, int ntriangles, std::string vertex_shader_path, std::string fragment_shader_path) {
    this->cx            = 0.f;
    this->cy            = 0.f;
    this->cx1           = cx;
    this->cy1           = cy;
    this->displacement  = glm::vec3(cx1, cy1, 0.f);
    this->r             = r;
    this->ntriangles    = ntriangles;

    this->acceleration  = glm::vec3(0.f);
    this->speed         = glm::vec3(0.f); //glm::vec3(0.3f, 0.4f, 0.f);
    this->transform     = glm::mat4(1.f);
    this->view_project_mat = glm::mat4(1.f);

    float h = r;
    float w = r;

    float s1[4][2] = {{-1.f, 1.f}, {1.f, 1.f}, {1.f, -1.f}, {-1.f, -1.f}};
    float s2[4][2] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    vertices = new float[20];
    indices  = new int[6];

    for (int i=0; i<4; i++) {
        vertices[5*i+0] = this->cx + s1[i][0] * w;
        vertices[5*i+1] = this->cy + s1[i][1] * h;
        vertices[5*i+2] = 0.00f;
        vertices[5*i+3] = s2[i][0];
        vertices[5*i+4] = s2[i][1];
    }

    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 0; indices[4] = 2; indices[5] = 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*20, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texture_width, texture_height, texture_channels;
    unsigned char* texture_data = stbi_load("res/textures/ball1.png", &texture_width, &texture_height, &texture_channels, 0);


    //shader_ptr = new Shader("shaders/table.vs", "shaders/table.fs");
    shader_ptr = new Shader("shaders/cue.vs", "shaders/cue.fs");
    std::cout << texture_width << " " << texture_height << " " << texture_channels << std::endl;
    if (texture_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Fail to load texture from: " << "res/textures/table.png" << std::endl; 
    }

    stbi_image_free(texture_data);
    shader_ptr->use();
    shader_ptr->setInt("OurTexture", 0);
    pocket_ptr = NULL;
    this->mass = 0.160f;
}


void Ball2D::render() {
    std::cout << this->cx1 << " " << this->cy1 << " " << this->displacement.x << " " << this->displacement.y << std::endl;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader_ptr->use();
    //std::cout << cx1 << " " << cy1 << transform[3][0] << " " << transform[3][1] << std::endl;
    shader_ptr->setMat4("transform", view_project_mat*transform);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Ball2D::update(float deltaTime) {
    if (glm::length(speed) > 0) {
        acceleration = -(sliding_fraction * mass * 0.4f) * glm::normalize(speed);
        glm::vec3 speed1        = speed + acceleration * deltaTime;
        if (glm::dot(speed1, speed) < 0) {
            deltaTime = glm::length(speed) / glm::length(acceleration);
        }
        displacement = displacement + speed * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
        speed = speed1;
        cx1 = displacement.x;
        cy1 = displacement.y;
    }
    transform[3][0] = cx1 - cx;
    transform[3][1] = cy1 - cy;
}

bool Ball2D::collide(const LineSegment2D &segment) {
    glm::vec2 C = glm::vec2(cx1, cy1);
    glm::vec2 A = segment.p0 - C;
    glm::vec2 B = segment.p1 - segment.p0;

    float a = glm::dot(B, B);
    float b = 2 * glm::dot(A, B);
    float c = glm::dot(A, A) - r * r;

    float d = b * b - 4 * a * c;
    //std::cout << d << std::endl;
    if (d < 0) {
        return false;
    }

    float t1 = (-b + sqrtf(d)) / (2 * a);
    float t2 = (-b - sqrtf(d)) / (2 * a);

    return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
}

bool Ball2D::isFalling(const Circle2D &pocket) {
    glm::vec2 v(cx1 - pocket.cx1, cy1 - pocket.cy1);
    return glm::dot(v, v) < pocket.r * pocket.r;
}

bool Ball2D::collide(const Ball2D &ball) {
    return glm::length(glm::vec2(ball.cx1, ball.cy1)-glm::vec2(cx1, cy1)) < (r+ball.r+1e-6);
}


float Ball2D::getCollideTime(const Ball2D &ball, float deltaTime, float absoluteTolerance=1e-5) {
    float l = 0, r = deltaTime;
    glm::vec3 A = glm::vec3(cx1, cy1, cz) - glm::vec3(ball.cx1, ball.cy1, ball.cz); //relative displacement
    glm::vec3 B = speed - ball.speed;
    glm::vec3 C = 0.5f * (acceleration - ball.acceleration);
    float d = ball.r + r;
    while (true) {
        float m = (l + r) / 2.f;
        float t = glm::length(A + B * m + C * m * m) - d;
        if (t < 0.) {
            r = m;
        } else if (t > absoluteTolerance) {
            l = m;
        }  else {
            return m;
        }
    }
}