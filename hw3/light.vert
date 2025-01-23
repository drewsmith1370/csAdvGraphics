#version 430 compatibility

uniform mat4 modelViewMat;
uniform vec3 LightPos;

layout (location=0) in vec3 Pos;
layout (location=1) in vec3 Nrm;
layout (location=2) in vec3 Tan;
layout (location=3) in vec2 Tex;
layout (location=4) in vec3 Col;

out mat3 TNB;
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
    vec3 normal    = normalize(Nrm);
    vec3 tangent   = normalize(Tan);
    vec3 bitangent = cross(normal,tangent);
    // Create Matrix
    TNB = mat3(tangent,normal,bitangent);
    // Rotate to modelview
    TNB = gl_NormalMatrix * TNB;

    //  Eye position
    View  = -P.xyz;
    // Tex Coords
    vTex = Tex;

    vec4 pos = gl_ProjectionMatrix * P;
    gl_Position = pos;
    vCol = Col;
}