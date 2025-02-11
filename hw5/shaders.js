export const vertSource = `#version 300 es
precision highp float;

layout(std140) uniform SharedUniformBlock {
    mat4 uView;
    mat4 uProj;
    vec3 uLightDir;
    float uTime;
};

layout (location=0) in vec3 aPos;

out vec3 vPos;

void main() {
    gl_Position = uProj * uView * vec4(aPos,1.0);
    vPos = aPos;
}
`;

export const fragSource = `#version 300 es
precision highp float;

in vec3 vPos;

layout (location=0) out vec4 FragColor;

void main() {
    FragColor = vec4(vPos, 1.0);
}
`;
