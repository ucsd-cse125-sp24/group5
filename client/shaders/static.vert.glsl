#version 330 core

layout (location = 0) in vec3 myvertex;
layout (location = 1) in vec3 mynormal;
layout (location = 2) in vec2 texcoord;


out vec3 fragNormal;
out vec2 fragTexcoord;

uniform mat4 modelview;
uniform int hasDiffuseTexture;
uniform vec3 diffuseColor;

void main() {
    // Perform vertex transformation
    gl_Position = modelview * vec4(myvertex, 1);

    // Pass interpolated values to fragment shader
    fragNormal = mynormal;
    fragTexcoord = texcoord;
}
