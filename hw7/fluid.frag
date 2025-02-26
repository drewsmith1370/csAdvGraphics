#version 450

layout (std140, binding=0) uniform MouseUniformBuffer {
    vec2 mozCoord;
} mozUbo;
uniform sampler2D FluidTex;

in vec2 vTex;

out vec4 FragColor;

void main() {
    vec2 dif = vTex - mozUbo.mozCoord;
    float dist2 = dif.x*dif.x + dif.y*dif.y;

    vec4 col = texture(FluidTex,vTex);
    col.rg = col.rg * .5 + .5;
    // FragColor = col.rrrr;
    FragColor = col.aaaa;
}