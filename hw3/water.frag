#version 430

uniform float Time;

in mat3 TNB;
in vec3 Light;
in vec3 View;
in vec2 vTex;
in vec3 vCol;

out vec4 FragColor;

// Make these Uniforms later ig, but it would be best to make more organized objects in main first
const float Ambient = .4;
const float Diffuse = .8;
const float Spectral = 1;
// Wave props
const float vel = 1;
const float lax = 2;
const float lay = 2;

vec4 waveNormal(vec2 tex) {
   // Calculate theta
   float th = lax * tex.x;
   float ph = lay * tex.y;
   float travel = vel * Time;

   float h = 0;
   float u = 0;
   float v = 0;

   // impulse wave = 

   for (int i=2;i<10;i++) {
      float c = cos(i * th + (i-2) * ph + travel)/i;
      h += c;
      u += th * c;
      v += ph * c;
   }

   vec3 nrm = vec3(
      -u,
      1,
      -v
   );

   return vec4(normalize(nrm),h);
}

vec3 blinn(vec3 nrm, float height)
{
   //  N is the object normal
   vec3 N = normalize(nrm);
   //  L is the light vector
   vec3 L = normalize(Light);
   //  V is the view vector
   vec3 V = normalize(View);

   //  Emission and ambient color
   float intensity = Ambient;// * (height*.5+.5);


   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   //  Add diffuse
   if (Id >= 0.0) intensity += Id * Diffuse;
   //  The half vectors
   vec3 H = normalize(V+L);
   //  Specular is cosine of reflected and view vectors
   float Is = dot(H,N) * Spectral;
   if (Is>0.0) Is = pow(Is,30);
   else Is = 0;

   //  Return sum of color components
   return intensity * vCol + Is * vec3(.9,1,.9);
}

void main() {
   vec4 tmp = waveNormal(vTex);
   float h = tmp.w;
   vec3 nrm = TNB * vec3(tmp);

   FragColor = vec4(blinn(nrm,h),1);
   // FragColor = vec4(nrm*.5+.5,1); // debug normals
}