#version 430 compatibility

// uniform mat4 modelViewMat;
// uniform vec3 LightPos;

layout (location=0) in vec3 Pos;
layout (location=1) in vec3 Nrm;
layout (location=2) in vec2 Tex;
layout (location=3) in vec3 Col;

out vec3 vNrm;
out vec3 Light;
out vec3 View;
out vec2 vTex;
out vec3 vCol;

void main() {
    //
    //  Lighting values needed by fragment shader
    //
    //  Vertex location in modelview coordinates
    vec4 P = gl_ModelViewMatrix * vec4(Pos,1);
    //  Light position
    Light  = gl_LightSource[0].position.xyz - P.xyz;
    //  Find TNB Vectors
    vNrm = gl_NormalMatrix * normalize(Nrm);

    //  Eye position
    View  = -P.xyz;
    // Tex Coords
    vTex = Tex;

    gl_Position = gl_ProjectionMatrix * P;
    vCol = Col;
}