#version 330 core

in vec3 fragNormal;
in vec2 fragTexcoord;

uniform sampler2D tex ;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;
uniform int hasDiffuseTexture;
uniform vec3 diffuseColor;

out vec4 fragColor;

void main() {
    vec3 transformedNormal = (inverse(transpose(model)) * vec4(fragNormal, 1)).xyz;
    float mult = max(0, dot(transformedNormal, normalize(vec3(1, 1, 0))));
    if (hasDiffuseTexture != 0) {
        vec4 texColor = texture(tex, fragTexcoord);
        fragColor = texColor;
    } else {
        fragColor = vec4(mult * diffuseColor, 1);
    }
}
