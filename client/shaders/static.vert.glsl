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

uniform bool isAnimated;
uniform mat4 boneTransform[100]; // Up to 100 bones per model

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main() {
    // Perform vertex transformation
    if (isAnimated) {
        vec4 totalPosition = vec4(0);
        for (int i = 0; i < 4; i++) {
            if (boneidx[i] == -1) {
                break;
            } else if (boneidx[i] >= 100) {
                // here for debug purposes
                totalPosition = vec4(vertex, 1) + vec4(0, 3, 0, 0);
                break;
            }
            vec4 localPos = boneTransform[boneidx[i]] * vec4(vertex, 1);
            totalPosition += boneweight[i] * localPos;
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
