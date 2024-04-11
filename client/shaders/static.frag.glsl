#version 330

in vec3 fragNormal;
in vec2 fragTexcoord;
// You will certainly need this matrix for your lighting calculations
uniform mat4 modelview;

out vec4 fragColor;

void main() {
    float mult = max(0, dot(fragNormal, normalize(vec3(1, 1, 0))));
    fragColor = 0.2 + vec4(mult, mult, mult, 1.0f);
}
