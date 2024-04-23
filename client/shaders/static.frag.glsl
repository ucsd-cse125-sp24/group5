#version 330 core

in vec3 fragNormal;
in vec2 fragTexcoord;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;
uniform vec3 cameraPosition;

uniform int hasDiffuseMap;
uniform sampler2D diffuseTexture;
uniform vec3 diffuseColor;

uniform int hasSpecularMap;
uniform sampler2D specularTexture;
uniform vec3 specularColor;

uniform vec3 emissiveColor;

uniform vec3 ambientColor;

uniform int hasBumpMap;
uniform sampler2D bumpTexture;

uniform int hasDisplacementMap;
uniform sampler2D displacementTexture;

uniform int hasRoughMap;
uniform sampler2D roughTexture;
uniform vec3 roughColor;

out vec4 fragColor;

void main() {
    vec3 transformedNormal = (inverse(transpose(model)) * vec4(fragNormal, 1)).xyz;
    float mult = max(0, dot(transformedNormal, normalize(vec3(1, 1, 0))));
    if (hasDiffuseMap != 0) {
        vec4 texColor = texture(diffuseTexture, fragTexcoord);
        fragColor = texColor;
    } else {
        fragColor = vec4(mult * diffuseColor, 1);
    }
}
