#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in ivec4 boneidx; // Up to 4 bones can influence a single vertex
layout (location = 4) in vec4 boneweight;

out vec4 projectedFragPosition;
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexcoord;

uniform mat4 boneTransform[100]; // Up to 100 bones per model

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 totalPosition = vec4(0);
    for (int i = 0; i < 4; i++) {
        if (boneidx[i] == -1) {
            continue;
        } else if (boneidx[i] >= 100) {
            totalPosition = vec4(vertex, 1);
            break;
        }
        totalPosition += boneweight[i] * boneTransform[boneidx[i]] * vec4(vertex, 1);
    }
    projectedFragPosition = perspective * view * totalPosition;
    gl_Position = projectedFragPosition;

    fragPosition = vertex;
    fragNormal = normal;
    fragTexcoord = texcoord;
    // TODO: fix normals during animations...

}
