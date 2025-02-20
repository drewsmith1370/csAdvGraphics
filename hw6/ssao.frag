// Copy verbatim
#version 450 compatibility

uniform sampler2D img;
uniform sampler2D zbuf;

uniform float dx;
uniform float dy;

const int kernelSize = 32;
const float kernelRadius = .001;

vec3 kernel[kernelSize];

// Sample the depth buffer at current pixel plus given offset
float sampleZ(float dx,float dy) {
   return texture2D(zbuf,gl_TexCoord[0].st+vec2(dx,dy)).r;
}

// Hash fxn
float permhash(vec2 p) {
  float hash = mod(p.y*p.x - p.y, 289.0);
  hash = mod((hash*51.0 + 2.0)*hash + p.x, 289.0);
  hash = mod((hash*34.0 + 10.0)*hash, 289.0);
  return hash / 289.0;
}

// Create a new random number in range lo to hi
int randint=0;
float randInRange(float lo, float hi) {
    float hash = permhash(gl_TexCoord[0].st+randint++);
    return hash * (hi - lo) + lo;
}

float lerp(float a, float b, float t) {
    return (a - b) * t + a;
}

// Use nearby depth information to generate an approximate TNB matrix
mat3 getTNB(float depth) {
    float z1 = sampleZ(dx,0);
    float z2 = sampleZ(0,dy);

    float dz1 = depth - z1;
    float dz2 = depth - z2;

    vec3 tan = normalize(vec3(dx,0,dz1));
    vec3 bit = normalize(vec3(0,dy,dz2));
    vec3 nrm = cross(tan,bit);
    
    return mat3(tan,nrm,bit);
}

// Generate the kernel used for occlusion samples
void genKernel(mat3 tnb) {
    // Random Rotation about z
    float th = permhash(gl_TexCoord[0].st) * 1;
    mat3 randRot = mat3(
        vec3(cos(th),sin(th),0),
        vec3(-sin(th),cos(th),0),
        vec3(0,0,1)
    );
    // Create random directions in a hemisphere facing +z
    for (int i=0; i< kernelSize; i++) {
        // Generate hemisphere of random points
        kernel[i] = vec3(
            randInRange(-1,1),
            randInRange(-1,1),
            randInRange(0,1)
        );
        // Rotate hemisphere to TNB matrix
        kernel[i] = tnb * randRot * kernel[i];
        // Normalize the vectors
        kernel[i] = normalize(kernel[i]);
        // Scale the vectors randomly
        float scale = randInRange(0,1);
        // Use quadratic smoothing for higher density towards edge
        scale = 1 - scale*scale;
        // Apply scaling
        kernel[i] = scale * kernel[i];
    }
}

// Calculate the occlusion of the given fragment
float occlusion(float depth) {
    // Scale kernel size based on how far it is from camera
    float radius = kernelRadius / (.5 - depth*.5);
    float occlusion = 0;
    int samplesUsed = 0;
    // Cast each ray in the kernel
    for(int i=0; i < kernelSize; i++) {

        vec3 ray = radius * kernel[i];
        
        float rayDepth = sampleZ(ray.x,ray.y);
        float difference = depth + ray.z - rayDepth;

        if (difference < .01) {
            occlusion += step(kernelRadius,difference) * 1.5;
            samplesUsed++;
        }
    }
    return occlusion / float(samplesUsed);
}

void main()
{
    // Texture info
    vec4 col = texture2D(img,gl_TexCoord[0].st);
    float depth = sampleZ(0,0);
    // Use depth info to generate TNB matrix
    mat3 tnb = getTNB(depth);
    // Generate kernel
    genKernel(tnb);
    // Calculate fragment occlusion


    // vec3 nrm = getTNB(depth)[1];
    // gl_FragColor = vec4(nrm,1);
    float occ = occlusion(depth) * 1.1;
    gl_FragColor = col - 1*vec4(vec3(occ),0);// + vec4(.1,0,0,0);
}
