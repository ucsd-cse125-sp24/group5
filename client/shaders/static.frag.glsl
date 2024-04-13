#version 330 core

in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D tex ;
// You will certainly need this matrix for your lighting calculations
uniform mat4 modelview;

out vec4 fragColor;

void main() {
    float mult = max(0, dot(fragNormal, normalize(vec3(1, 1, 0))));
    vec4 texColor = texture(tex, fragTexcoord);
    fragColor = 0.2 * texColor + mult * texColor;
}
