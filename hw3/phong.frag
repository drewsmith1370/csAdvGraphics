//  Per-pixel Phong lighting
//  Fragment shader
#version 120

varying vec3 View;
varying vec3 Light;
varying vec3 Normal;
varying vec4 Ambient;
uniform sampler2D tex;

vec4 phong()
{
   //  N is the object normal
   vec3 N = normalize(Normal);
   //  L is the light vector
   vec3 L = normalize(Light);

   //  Emission and ambient color
   vec4 color = vec4(.1,.1,.1,1);

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      color += Id*vec4(.6,.6,.6,1);
      //  R is the reflected light vector R = 2(L.N)N - L
      vec3 R = reflect(-L,N);
      //  V is the view vector (eye vector)
      vec3 V = normalize(View);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(R,V);
      if (Is>0.0) color += pow(Is,gl_FrontMaterial.shininess)*vec4(.8,.8,.8,1);
   }

   //  Return sum of color components
   return color;
}

void main()
{
   gl_FragColor = phong();// * texture2D(tex,gl_TexCoord[0].xy);
   // gl_FragColor = vec4(Normal,1);
}
