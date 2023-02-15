#include "circle2d.hpp"

Circle2D::Circle2D(float cx, float cy, float r, int ntriangles, std::string vertex_shader_path, std::string fragment_shader_path) {
    this->cx            = cx;
    this->cy            = cy;
    this->cx1           = cx;
    this->cy1           = cy;
    this->r             = r;
    this->ntriangles    = ntriangles;
    this->shader_ptr    = new Shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());
    
    // std::cout << "***********************************\n";
    // std::cout << cx << " " << cy << " " << cx1 << "  " << cy1 << std::endl;
    // std::cout << "***********************************\n";
    // triangulate the circle
    this->vertices      = new float[(ntriangles + 1) * 3];
    this->indices       = new int[ntriangles*3];
    this->acceleration  = glm::vec3(0.f);
    this->speed         = glm::vec3(0.f); //glm::vec3(0.3f, 0.4f, 0.f);
    this->transform     = glm::mat4(1.f);
    this->view_project_mat = glm::mat4(1.f);
    //std::cout << this->transform << std::endl;

    triangulate();
    // std::cout << sizeof this->vertices  << " " << sizeof(this->indices) << std::endl;

    // setup buffers for opengl
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(ntriangles+1)*3, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*ntriangles*3, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    pocket_ptr = NULL;
    this->mass = 0.160f;
}

void Circle2D::triangulate() {
    // create the mesh of a circle on the fly
    int i = 0;
    for (i = 0; i < this->ntriangles; i++) {
        float theta = i * 2 * M_PI / this->ntriangles;
       
        float x = this->r * cosf(theta);
        float y = this->r * sinf(theta);

        vertices[3*i] = cx + x;
        vertices[3*i+1] = cy + y;
        vertices[3*i+2] = 0.f;
    }
    vertices[3*i] = cx;
    vertices[3*i+1] = cy;
    vertices[3*i+2] = 0;

    for (i = 0; i < ntriangles; i++) {
        indices[3*i] = ntriangles;
        indices[3*i+1] = i;
        indices[3*i+2] = (i+1) % ntriangles;
    }
}

void Circle2D::render() {
    shader_ptr->use();
    shader_ptr->setMat4("transform", view_project_mat*transform);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, ntriangles*3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Circle2D::update(float deltaTime) {
    // this is not physically accurate!

    if (glm::length(speed) > 0) {
        acceleration = -(sliding_fraction * mass * 0.2f) * glm::normalize(speed);
        glm::vec3 speed1        = speed + acceleration * deltaTime;
        if (glm::dot(speed1, speed) < 0) {
            deltaTime = glm::length(speed) / glm::length(acceleration);
        }
        displacement = displacement + speed * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
        speed = speed1;
        cx1 = cx + displacement.x;
        cy1 = cy + displacement.y;
    }





    // see if the circle is out of table
    // cx1 = cx1 + deltaTime * speed.x;
    // cy1 = cy1 + deltaTime * speed.y;
    // //std::cout << deltaTime << " " << cx1 << " " << cy1 << std::endl;

    // if ((cx1 - r < -0.99f) || (cx1 + r > 0.99f) ||
    //     (cy1 - r < -0.99f) || (cy1 + r > 0.99f)) {
    //     // fix speed 
    //     // can we use a reflection matrix here?
    //     if ((cx1 > 0.99f-r) || (cx1 < -0.99f+r)) {
    //         speed.x = 0.86 * -speed.x;
    //     }
    //     if ((cy1 > 0.99f-r) || (cy1 < -0.99f+r)) {
    //         speed.y = 0.86 * -speed.y;
    //     }
    //     // fix center
    //     cx1 = std::min(std::max(cx1, -0.99f+r), 0.99f-r);
    //     cy1 = std::min(std::max(cy1, -0.99f+r), 0.99f-r);
    // }


    transform[3][0] = cx1 - cx;
    transform[3][1] = cy1 - cy;
}

bool Circle2D::collide(const LineSegment2D &segment) {
    glm::vec2 C = glm::vec2(cx1, cy1);
    glm::vec2 A = segment.p0 - C;
    glm::vec2 B = segment.p1 - segment.p0;

    float a = glm::dot(B, B);
    float b = 2 * glm::dot(A, B);
    float c = glm::dot(A, A) - r * r;

    float d = b * b - 4 * a * c;
    if (d < 0) {
        return false;
    }

    float t1 = (-b + sqrtf(d)) / (2 * a);
    float t2 = (-b - sqrtf(d)) / (2 * a);

    return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
}

bool Circle2D::isFalling(const Circle2D &pocket) {
    glm::vec2 v(cx1 - pocket.cx1, cy1 - pocket.cy1);
    return glm::dot(v, v) < pocket.r * pocket.r;
}



