#version 430 compatibility

layout (location=0) in vec3 Pos;

void main() {
    vec4 P = gl_ModelViewMatrix * vec4(Pos,1);
    gl_Position = gl_ProjectionMatrix * P;
}