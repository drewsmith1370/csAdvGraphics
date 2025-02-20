#version 430

in vec3 vNrm;
in vec3 Light;
in vec3 View;
in vec2 vTex;
in vec3 vCol;

out vec4 FragColor;

void main() {
   FragColor = vec4(vCol,1);
}