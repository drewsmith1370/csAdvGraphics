#version 450

layout (local_size_x = 32, local_size_y = 32) in;

layout (std140, binding=0) uniform MouseUniformBuffer {
    vec2 mozCoord;
    float deltaTime;
    bool click;
} mozUbo;
// r, g, b channels are velocity vectors, a is density
layout (rgba32f, binding=0) uniform image2D FluidTex;

// const float grav = 0.98;
const float diffuse_rate = 100;
const float advect_rate  = .01;

vec4 getFluidInfo(ivec2 texel) {
    return imageLoad(FluidTex, texel);
}

vec4 diffusion(vec4 fluidInfo, ivec2 texel) {
    // Every frame swap densities with surrounding neighbors
    float dif = diffuse_rate * mozUbo.deltaTime;
    vec3 vel = fluidInfo.xyz;
    float newDensity = fluidInfo.a + dif *(getFluidInfo(texel + ivec2( 0,-1)).a + 
                                            getFluidInfo(texel + ivec2(-1, 0)).a +
                                            getFluidInfo(texel + ivec2( 0, 1)).a +
                                            getFluidInfo(texel + ivec2( 1, 0)).a - 4*fluidInfo.a);

    return vec4(vel,newDensity);
}

ivec2 wrapEdges(ivec2 v) {
    v.x = int(mod(v.x, 512));
    v.y = int(mod(v.y, 512));
    return v;
}

// Solve advection backwards (what points density value will end up in this one?)
vec4 advection(vec4 fluidInfo, ivec2 texel) {
    // Use velocity to travel backwards
    vec2 advCoord = vec2(texel) - fluidInfo.xy * mozUbo.deltaTime * advect_rate;
    // find the surrounding coordinates
    ivec2 bl = ivec2(advCoord);
    ivec2 br = ivec2(bl.x+1,bl.y);
    ivec2 ul = ivec2(bl.x,bl.y+1);
    ivec2 ur = ivec2(bl.x+1,bl.y+1);
    wrapEdges(bl);
    wrapEdges(br);
    wrapEdges(ul);
    wrapEdges(ur);
    // Fractional part used for interpolating between the texel values
    vec2 interpHi = fract(advCoord);
    vec2 interpLo = 1 - interpHi;
    // Sample the points surrounding the actual point and interpolate for new value
    vec4 newVal = getFluidInfo(bl) * interpLo.x*interpLo.y +
                  getFluidInfo(br) * interpHi.x*interpLo.y +
                  getFluidInfo(ul) * interpLo.x*interpHi.y +
                  getFluidInfo(ur) * interpHi.x*interpHi.y;

    fluidInfo.a = newVal.a;
    return fluidInfo;
}

vec4 addSource(vec4 info, ivec2 texel) {
    vec2 dif = mozUbo.mozCoord - vec2(texel) / 512.0;
    float dst = dif.x*dif.x + dif.y*dif.y;
    if (dst < .00001) {
        info.xy = vec2(0);
        info.a += .1;
    }
    vec2 loc = vec2(texel - 256);
    loc = normalize(vec2(loc.y,-loc.x));
    info.xy = loc;
    return info;
}

void main() {
    // Get invocation texel coord
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    memoryBarrier();

    vec4 fluidInfo = getFluidInfo(texelCoord);
    // Diffusion
    vec4 newInfo = diffusion(fluidInfo, texelCoord);
    newInfo = advection(newInfo, texelCoord);
    if (mozUbo.click) newInfo = addSource(newInfo, texelCoord);
    // Store new values
    barrier();
    imageStore(FluidTex, texelCoord, newInfo);
}
