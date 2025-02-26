#version 450

layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTex;

out vec2 vTex;

void main() {
    gl_Position = vec4(aPos,1);
    vTex = aTex;
}