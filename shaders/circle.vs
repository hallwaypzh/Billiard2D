#version 330 core

layout(location=0) in vec3 aPos;

out vec4 vertexColor;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(aPos, 1.f);
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
}