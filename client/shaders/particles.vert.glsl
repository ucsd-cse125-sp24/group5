#version 330 core

layout (location = 0) in vec3 vertex; // should be in world coords
layout (location = 1) in vec3 color;
layout (location = 2) in mat4 transform;

layout (location = 0) out position;
layout (location = 1) out color;
layout (location = 2) out transform;

uniform mat4 perspective;
uniform mat4 view;

uniform

void main() {
    // Let geometry shader handle transforming to camera view since we might convert this point
    // to another primitive
    gl_Position = vec4(vertex, 1);
}
