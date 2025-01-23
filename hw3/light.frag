#version 430

in mat3 TNB;
in vec3 Light;
in vec3 View;
in vec2 vTex;
in vec3 vCol;

out vec4 FragColor;

// Make these Uniforms later ig, but it would be best to make more organized objects in main first
const float Ambient = .1;
const float Diffuse = .8;
const float Spectral = 1;
// Wave props
const float time = 0;
const float vel = 1;
const float lax = .5;
const float lay = .2;

vec3 blinn(vec3 nrm)
{
   //  N is the object normal
   vec3 N = normalize(nrm);
   //  L is the light vector
   vec3 L = normalize(Light);
   //  V is the view vector
   vec3 V = normalize(View);

   //  Emission and ambient color
   float intensity = Ambient;

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      intensity += Id * Diffuse;
      //  The half vectors
      vec3 H = normalize(V+L);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(H,N) * Spectral;
      if (Is>0.0) intensity += pow(Is,20);
   }

   //  Return sum of color components
   return intensity * vCol;
}

void main() {
   vec3 nrm = TNB * vec3(0,1,0);

   FragColor = vec4(blinn(nrm),1);
}