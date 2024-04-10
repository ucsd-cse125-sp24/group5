#version 330

in vec3 mynormal;
in vec4 myvertex;
in vec2 texcoord;
// You will certainly need this matrix for your lighting calculations
uniform mat4 modelview;

out vec4 fragColor;

void main() {
    fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
