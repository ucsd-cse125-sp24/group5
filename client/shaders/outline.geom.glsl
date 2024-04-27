#version 330 core

// Input geometry primitive type
layout (triangles_adjacency) in

// Output geometry primitive type
layout (triangle_strip, max_vertices=8) out

in vec4 projectedFragPosition;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;
uniform int hasDiffuseTexture;
uniform vec3 diffuseColor;

void main() {

}
