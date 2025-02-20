// Blur (low-pass)
//   1 2 1
//   2 1 2   / 13
//   1 2 1
#version 120

uniform float dx;
uniform float dy;
uniform sampler2D img;

vec4 sample(float dx,float dy)
{
   return texture2D(img,gl_TexCoord[0].st+vec2(dx,dy));
}

void main()
{
   float one = 1.0/13.0;
   float two = 2.0/13.0;
   gl_FragColor = one*sample(-dx,+dy) + two*sample(0.0,+dy) + one*sample(+dx,+dy)
                + two*sample(-dx,0.0) + one*sample(0.0,0.0) + two*sample(+dx,0.0)
                + one*sample(-dx,-dy) + two*sample(0.0,-dy) + one*sample(+dx,-dy);
}
