#include "table2d.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Table2D::Table2D(const glm::mat4 &view_project_mat) {
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

    // for (int r=0; r<2; r++) {
    //     for (int c=0; c<2; c++) {
    //         std::cout << r << " " << c << " " << 2 * r + c << " " << vertices[5*(2*r+c)+0] << " " << vertices[5*(2*r+c)+1] << " " << vertices[5*(2*r+c)+3] << " "  << vertices[5*(2*r+c)+4] << std::endl;
    //     }
    // }

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
    texture_data = stbi_load("res/textures/billiard_table1.png", &texture_width, &texture_height, &texture_channels, 0);

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

    // set up boundaries of the table
    
    float pts[12] =   {-0.6613129770992366, 0.2912134809160305, 0.6613129770992366, 0.2912134809160305, -0.6193904580152672, 0.333136, 0.04192251908396949, 0.333136,-0.6193904580152672, -0.33316, 0.04192251908396949, -0.33316};
    float l = 0.5774679389312977;
    float l1  = 0.582426961832061;

    lines[0] = LineSegment2D(pts[0], pts[1], pts[0], pts[1]-l1); lines[0].layout = LEFT;
    lines[1] = LineSegment2D(pts[2], pts[3], pts[2], pts[3]-l1); lines[1].layout = RIGHT;
    lines[2] = LineSegment2D(pts[4], pts[5], pts[4]+l, pts[5]); lines[2].layout = TOP;
    lines[3] = LineSegment2D(pts[6], pts[7], pts[6]+l, pts[7]); lines[3].layout = TOP;
    lines[4] = LineSegment2D(pts[8], pts[9], pts[8]+l, pts[9]); lines[4].layout = BOTTOM;
    lines[5] = LineSegment2D(pts[10], pts[11], pts[10]+l, pts[11]); lines[5].layout = BOTTOM;

    this->view_project_mat = view_project_mat;
    pockets[0] = new Circle2D(lines[0].p0.x, lines[3].p0.y, lines[3].p0.x, 360, "shaders/hole.vs", "shaders/hole.fs");
    pockets[0]->setViewProjectMatrix(this->view_project_mat);
    pockets[0]->set_z(-0.05f);

    pockets[1] = new Circle2D(0., lines[3].p0.y, lines[3].p0.x, 720, "shaders/hole.vs", "shaders/hole.fs");
    pockets[1]->setViewProjectMatrix(this->view_project_mat);
    pockets[1]->set_z(-0.05f);

    pockets[2] = new Circle2D(lines[1].p0.x, lines[3].p0.y, lines[3].p0.x, 720, "shaders/hole.vs", "shaders/hole.fs");
    pockets[2]->setViewProjectMatrix(this->view_project_mat);
    pockets[2]->set_z(-0.05f);

    pockets[3] = new Circle2D(lines[0].p0.x, lines[4].p0.y, lines[3].p0.x, 720, "shaders/hole.vs", "shaders/hole.fs");
    pockets[3]->setViewProjectMatrix(this->view_project_mat);
    pockets[3]->set_z(-0.05f);

    pockets[4] = new Circle2D(0., lines[5].p0.y, lines[5].p0.x, 720, "shaders/hole.vs", "shaders/hole.fs");
    pockets[4]->setViewProjectMatrix(this->view_project_mat);
    pockets[4]->set_z(-0.05f);

    //std::cout << pockets[4]->vertices[3*720+2] << std::endl;

    pockets[5] = new Circle2D(lines[1].p0.x, lines[5].p0.y, lines[5].p0.x, 720, "shaders/hole.vs", "shaders/hole.fs");
    pockets[5]->setViewProjectMatrix(this->view_project_mat);
    pockets[5]->set_z(-0.05f);

}

void Table2D::render() {
    for (auto pocket:pockets) {
        pocket->render();
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader_ptr->use();
    // shader_ptr->setInt("ourTexture", texture);
    shader_ptr->setMat4("transform", view_project_mat);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}