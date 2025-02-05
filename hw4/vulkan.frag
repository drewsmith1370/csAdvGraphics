#version 450

layout(binding=0) uniform UniformBufferObject {
   mat4 model;  // Model matrix
   mat4 view;   // View matrix
   mat4 proj;   // Projection matrix
   mat4 norm;   // Normal matrix (passed as 4x4 to facilitate alignment)
   vec4 pos;    // Light position
   vec4 Ca;     // Light ambient
   vec4 Cd;     // Light diffuse
   vec4 Cs;     // Light specular
   vec4 Ks;     // Material specular
   float Ns;    // Material shininess
   } ubo;

layout(binding = 1) uniform sampler2D tex; // Texture sampler

layout(location=0) in mat3 TNB;  // Normal vector
layout(location=3) in vec3 Light; // Light vector
layout(location=4) in vec3 View;  // Eye vector
layout(location=5) in vec3 col;   // Color
layout(location=6) in vec2 t2d;   // Texture

layout(location=0) out vec4 fragColor; // Pixel color

vec3 waveNormal(vec2 tex) {
   // Calculate theta
   vec2 dif = tex - vec2(.5,.5);
   float dist2 = dif.x*dif.x + dif.y*dif.y;
   vec3 nrm=vec3(0,1,0);

   // if (dist2 < r2) {
      nrm = vec3(
         -dif.x*cos(200*dif.x*dif.x+200*dif.y*dif.y),
         1,
         -dif.y*cos(200*dif.x*dif.x+200*dif.y*dif.y)
      );
   // }

   return normalize(nrm);
}

void main()
{
   //  Normalize vectors and calculate reflection
   vec3 nrm = normalize(TNB[0].xyz);
   vec3 tan = normalize(TNB[1].xyz);
   vec3 bit = normalize(TNB[2].xyz);
   mat3 TNB = mat3(nrm,tan,bit);

   // Calculate texture normal
   nrm = waveNormal(t2d);
   vec3 N = TNB * nrm;

   vec3 L = normalize(Light);
   vec3 V = normalize(View);
   vec3 R = reflect(-L,N);
   //  Diffuse light is cosine of light and normal vectors
   float Id = max(dot(L,N) , 0.0);
   //  Specular is cosine of reflected and view vectors
   float Is = (Id>0.0) ? pow(max(dot(R,V),0.0) , ubo.Ns) : 0.0;

   //  Sum color types
   vec4 Kd = vec4(col,1);
   vec4 color = Kd*ubo.Ca + Id*Kd*ubo.Cd + Is*ubo.Ks*ubo.Cs;

   //  Modulate color by texture
   fragColor = color * texture(tex,t2d);
}
