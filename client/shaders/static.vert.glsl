#version 330

in vec3 mynormal;
in vec4 myvertex;
in vec2 texcoord;

out vec3 fragNormal;
out vec2 fragTexcoord;

uniform mat4 modelview;

void main() {
    // Perform vertex transformation
    gl_Position = modelview * myvertex;

    // Pass interpolated values to fragment shader
    fragNormal = mynormal;
    fragTexcoord = texcoord;
}
