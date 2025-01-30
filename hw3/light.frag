#version 430

in vec3 vNrm;
in vec3 Light;
in vec3 View;
in vec2 vTex;
in vec3 vCol;

out vec4 FragColor;

// Make these Uniforms later ig, but it would be best to make more organized objects in main first
const float Ambient = .1;
const float Diffuse = .5;
const float Spectral = .1;
// Wave props
const float time = 0;
const float vel = 1;
const float lax = .5;
const float lay = .2;

vec4 phong()
{
   //  N is the object normal
   vec3 N = normalize(vNrm);
   //  L is the light vector
   vec3 L = normalize(Light);

   //  Emission and ambient color
   float intensity = Ambient;

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      intensity += Id*Diffuse;
      //  R is the reflected light vector R = 2(L.N)N - L
      vec3 R = reflect(-L,N);
      //  V is the view vector (eye vector)
      vec3 V = normalize(View);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(R,V);
      if (Is>0.0) intensity += pow(Is,10)*Spectral;
   }

   //  Return sum of color components
   return vec4(intensity * vCol,1);
}


void main() {
   FragColor = phong();
   // FragColor = vec4(vNrm,1);
}