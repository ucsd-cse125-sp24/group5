#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in ivec4 boneidx;
layout (location = 4) in vec4 boneweight;

out vec4 projectedFragPosition;
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexcoord;

const int max_bones = 100;
const int max_bone_influence = 4;

uniform bool isAnimated;
uniform mat4 boneTransform[max_bones]; // Up to 100 bones per model

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main() {
    // Perform vertex transformation
    if (isAnimated) {
        vec4 totalPosition = vec4(0);
        for (int i = 0; i < max_bone_influence; i++) {
            if (boneidx[i] == -1) {
                continue;
            } else if (boneidx[i] >= max_bones) {
                // here for debug purposes
                totalPosition = vec4(vertex, 1);
                break;
            }
            vec4 localPos = boneTransform[boneidx[i]] * vec4(vertex, 1);
            totalPosition += boneweight[i] * localPos;
        }
        if (boneidx[0] == -1) {
            totalPosition = vec4(vertex, 1);
        }

        projectedFragPosition = perspective * view * model * totalPosition;
        gl_Position = projectedFragPosition;

        fragPosition = vertex;
        fragNormal = normal;
        fragTexcoord = texcoord;
    } else {
        projectedFragPosition = perspective * view * model * vec4(vertex, 1);
        gl_Position = projectedFragPosition;

        // Pass interpolated values to fragment shader
        fragPosition = vertex;
        fragNormal = normal; // Transform normal according to model transformation
        fragTexcoord = texcoord;
    }
}
