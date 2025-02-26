#version 450

layout (std140, binding=0) uniform MouseUniformBuffer {
    vec2 mozCoord;
} mozUbo;
layout (binding=0) uniform sampler2D FluidTex;
layout (binding=1) uniform sampler2D DivTex;

in vec2 vTex;

out vec4 FragColor;

void main() {
    vec2 dif = vTex - mozUbo.mozCoord;
    float dist2 = dif.x*dif.x + dif.y*dif.y;

    // vec4 col = texture(DivTex, vTex).rrrr;
    vec4 col = texture(FluidTex,vTex);
    col.rg = col.rg * .5 + .5;
    // FragColor = col.aaaa;
    FragColor = col.rgba;
}