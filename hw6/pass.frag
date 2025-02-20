// Copy verbatim
#version 120

uniform sampler2D img;
uniform sampler2D zbuf;

void main()
{
    vec4 iCol = texture2D(img,gl_TexCoord[0].st);
    gl_FragColor = vec4(vec3(iCol),1);
}
