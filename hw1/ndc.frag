#version 120
varying vec4 pos;
void main() {gl_FragColor = vec4(pos.xyz / pos.w / 2 + .5,1);}