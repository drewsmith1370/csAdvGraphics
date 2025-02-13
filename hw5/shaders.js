export const vertSource = `#version 300 es
precision highp float;

layout(std140) uniform SharedUniformBlock {
    mat4 uView;
    mat4 uProj;
    vec3 uLightDir;
    float uTime;
};
layout(std140) uniform SpecificUniformBlock {
    mat4 uModel;
};

layout (location=0) in vec3 aPos;

out vec3 vNrm;
out vec3 vLdir;
out vec3 vView;

void main() {
    // Set position
    vec4 pos = uProj * uView * uModel * vec4(aPos,1.0);
    gl_Position = pos;
    // Set normals
    vNrm = mat3(uProj * uModel) * aPos;
    // Set light direction and view vector
    vLdir = uLightDir;
    vView = -vec3(pos);
}
`;

export const fragSource = `#version 300 es
precision lowp float;

in vec3 vNrm;
in vec3 vLdir;
in vec3 vView;

layout (location=0) out vec4 FragColor;

vec4 phong()
{
   //  N is the object normal
   vec3 N = normalize(vNrm);
   //  L is the light vector
   vec3 L = normalize(vLdir);

   //  Emission and ambient color
   vec4 color = vec4(.3,0,0,1);

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      color += Id*vec4(.5,0,0,1);
      //  R is the reflected light vector R = 2(L.N)N - L
      vec3 R = reflect(-L,N);
      //  V is the view vector (eye vector)
      vec3 V = normalize(vView);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(R,V);
      if (Is>0.0) color += pow(Is,1.0)*vec4(.5,.5,.5,.5);
   }

   //  Return sum of color components
   return color;
}

void main() {
    FragColor = phong();
}
`;

export const bgVertSrc = `#version 300 es
precision highp float;
layout (location=0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos.xy,.9,1);
}
`;

export const bgFragSrc = `#version 300 es
precision mediump float;

layout (location=0) out vec4 FragColor;

void main() {
    FragColor = vec4(0,0,0,0);
}
`;
