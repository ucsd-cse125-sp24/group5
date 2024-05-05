#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

out vec4 projectedFragPosition;
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexcoord;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main() {
    // Perform vertex transformation
    projectedFragPosition = perspective * view * model * vec4(vertex, 1);
    gl_Position = projectedFragPosition;

    // Pass interpolated values to fragment shader
    fragPosition = vertex;
    fragNormal = normal; // Transform normal according to model transformation
    fragTexcoord = texcoord;
}
