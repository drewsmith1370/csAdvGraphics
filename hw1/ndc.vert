#version 120
varying vec4 pos; 
void main() {pos = gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;}