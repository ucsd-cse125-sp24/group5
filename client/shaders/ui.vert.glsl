#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

// uniform float aspectRatio;
uniform mat4 projection;

uniform mat3 trans;

void main() {
    vec3 pos = vec3(position, 1.0);
    pos = trans * pos;  // scale, translation
    
    gl_Position = projection * vec4(pos.xy, 0, 1);  // ortho projection
    TexCoord = aTexCoord;
}
