#include "cue2d.hpp"

#include "stb_image.h"

Cue2D::Cue2D(const glm::mat4 &view_project_mat) {
    this->view_project_mat = view_project_mat;

    float h = 0.591f;
    float w = 0.009f;

    float s1[4][2] = {{-0.5f, 0.f}, {0.5f, 0.f}, {0.5f, -1.f}, {-0.5f, -1.f}};
    float s2[4][2] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    vertices = new float[20];
    indices  = new int[6];

    for (int i=0; i<4; i++) {
        vertices[5*i+0] = s1[i][0] * w;
        vertices[5*i+1] = s1[i][1] * h;
        vertices[5*i+2] = -0.05f;
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
    texture_data = stbi_load("res/textures/cue1.png", &texture_width, &texture_height, &texture_channels, 0);


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
}

void Cue2D::render() {
    if (state != STOPPED) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader_ptr->use();
        shader_ptr->setMat4("transform", view_project_mat*translation_mat1*translation_mat0*rotation_mat);
        // std::cout << "alpha" << " " << alpha << std::endl;
        shader_ptr->setFloat("alpha", alpha);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Cue2D::update(float delta_time) {
    if (state == FIRING) {
        
        glm::mat3 cue_state(acceleration, speed, displacement);
        glm::mat3 mat(1.f, delta_time, 0.5 * delta_time * delta_time,
                      0.f,        1.f, delta_time,
                      0.f,        0.f, 1.f);
        cue_state =  glm::transpose(mat * glm::transpose(cue_state)); 

        acceleration = cue_state[0];
        speed        = cue_state[1];
        displacement = cue_state[2];
        // std::cout << "ACC " << acceleration.x << " " << acceleration.y << " " << acceleration.z << std::endl;
        // std::cout << "VEL " << speed.x << " " << speed.y << " " << speed.z << std::endl;
        // std::cout << "DIS " << displacement.x << " " << displacement.y << " " << displacement.z << std::endl;
        translation_mat0 = glm::translate(translation_mat0, displacement);
    }

    if (state == STOPPED) {
        fade_frame_cnt += 1;
        alpha = std::max(1.f-(1.f *fade_frame_cnt)/FADE_FRAMES, 0.f);
    }
}