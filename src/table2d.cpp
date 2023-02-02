#include "table2d.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Table2D::Table2D() {
    float w = 1.547f;
    float h = 0.886f;

    vertices = new float[20];
    indices  = new int[6];

    int s1[4][2] = {{-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
    float s2[4][2] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    for (int i=0; i<4; i++) {
        vertices[5*i+0] = s1[i][0] * w/2;
        vertices[5*i+1] = s1[i][1] * h/2;
        vertices[5*i+2] = -0.05f;
        vertices[5*i+3] = s2[i][0];
        vertices[5*i+4] = s2[i][1];
    }

    for (int r=0; r<2; r++) {
        for (int c=0; c<2; c++) {
            std::cout << r << " " << c << " " << 2 * r + c << " " << vertices[5*(2*r+c)+0] << " " << vertices[5*(2*r+c)+1] << " " << vertices[5*(2*r+c)+3] << " "  << vertices[5*(2*r+c)+4] << std::endl;
        }
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

    //load textures;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texture_width, texture_height, texture_channels;
    texture_data = stbi_load("res/textures/table.png", &texture_width, &texture_height, &texture_channels, 0);

    shader_ptr = new Shader("shaders/table.vs", "shaders/table.fs");
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

void Table2D::render() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader_ptr->use();
    // shader_ptr->setInt("ourTexture", texture);
    shader_ptr->setMat4("transform", view_project_mat);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}