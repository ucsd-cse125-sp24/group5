#version 330 core

layout (location = 0) in vec3 vertex; // should be in world coords
layout (location = 1) in vec4 color;
layout (location = 2) in mat4 transform;

out vec4 vertexColor;
out mat4 vertexTransform;

void main() {
    // Output in world coordinates
    gl_Position = vec4(vertex, 1.0); // this is the local position, not transformed yet
    vertexColor = color;
    vertexTransform = transform;
}
