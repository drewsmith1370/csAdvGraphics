// Copy verbatim
#version 120

uniform sampler2D img;
uniform sampler2D zbuf;

void main()
{
    vec4 iCol = texture2D(img,gl_TexCoord[0].st);
    float depth = texture2D(zbuf,gl_TexCoord[0].st).x;
    gl_FragColor = vec4(vec3(depth-.5),1);
}
