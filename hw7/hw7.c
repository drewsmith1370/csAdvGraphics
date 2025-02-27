#include "CSCIx239.h"

#define NUM_FILTERS 3

// Inputs
char keys[256] = {0};
int ua = 0, da = 0, la = 0, ra = 0;
// View
float dim = 2;
float asp = 1;
// Misc
GLFWwindow* window=NULL;
int width=800, height=600;
int shader[3] = {0};
GLuint fbuf[2] = {0};
GLuint texture[2] = {0};
GLuint mozUbo = 0;
float mx=0, my=0;
int src=0;
// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
int paused = 0;
// Objects
unsigned int objects[1] = {0};
int objIndices = 4;

// Vertex stuff
typedef struct Vertex {
	float Pos[3];
	float Tex[2];
} Vertex;
// UBO
typedef struct MouseUniformBuffer {
	float mozCoord[2];
	float mozVel[2];
	float deltaTime;
	int click;
	int source;
} MouseUniformBuffer;

struct Vertex SurfaceVertices[] = {
	{.Pos={-1,-1, 0}, .Tex={0,0}},
	{.Pos={-1, 1, 0}, .Tex={0,1}},
	{.Pos={ 1, 1, 0}, .Tex={1,1}},
	{.Pos={ 1,-1, 0}, .Tex={1,0}}
};

GLuint SurfIndices[] = {
	0,1,3 , 3,1,2
};

void updateMouseCoord(double newMozX, double newMozY, int width, int height) {
	double currentX = newMozX / width;
	double currentY = 1 - newMozY / height;
	double dx = currentX - mx;
	double dy = currentY - my;
	mx = currentX;
	my = currentY;

	float mubo[4] = {currentX, currentY, dx, dy};
	glBindBuffer(GL_UNIFORM_BUFFER,mozUbo);
	glBufferSubData(GL_UNIFORM_BUFFER,offsetof(MouseUniformBuffer,mozCoord),sizeof(float[4]),&mubo);
	return;
}

// Render and swap buffers
void display(GLFWwindow* window) {
	glBindFramebuffer(GL_FRAMEBUFFER,fbuf[0]);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	double xpos,ypos;
	glfwGetCursorPos(window,&xpos,&ypos);
	updateMouseCoord(xpos,ypos,width,height);

	// Computes
	// Dispatch density solver
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,texture[1]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture[0]);
	glUseProgram(shader[1]);
	glDispatchCompute(16, 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	// // Dispatch velocity solver
	// glUseProgram(shader[2]);
	// glDispatchCompute(16, 16, 1);
	// glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	//  Draw scene
	glUseProgram(shader[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	ErrCheck("scene");

	//  Revert to fixed pipeline
	glUseProgram(0);
	glDisable(GL_LIGHTING);
	//  Display parameters
	glWindowPos2i(5,5);
	Print("FPS:%d x:%.2f y:%.2f",FramesPerSecond(),mx,my);
	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	glfwSwapBuffers(window);
}

// Reshape callback
void reshape(GLFWwindow* window, int w, int h) {
	//  Get framebuffer dimensions (makes Apple work right)
	glfwGetFramebufferSize(window,&width,&height);
	double xpos,ypos;
	glfwGetCursorPos(window,&xpos,&ypos);
	if (mozUbo) updateMouseCoord(xpos,ypos,width,height);
	//  Ratio of the width to the height of the window
	asp = (height>0) ? (double)width/height : 1;
	//  Set the viewport to the entire window
	glViewport(0,0, width,height);
	//  Set projection
	Projection(0,asp,dim);
}

// Update time variables each frame
void updateTime() {
    // Update time and deltaTime
    double now = glfwGetTime();
    deltaTime = now - prevTime;
	// Increment time
	if (!paused) progTime += deltaTime;
	// Update previous
    prevTime = now;

	float dt = (float)deltaTime;
	glBufferSubData(GL_UNIFORM_BUFFER,offsetof(MouseUniformBuffer,deltaTime),sizeof(float),&dt);
}

// Handle new keys from glfw
void key(GLFWwindow* window,int key,int scancode,int action,int mods) {
    char setTo;
	// Figure out if key was pressed or released
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        setTo = 1;

		//  PageUp key - increase dim
		if (key==GLFW_KEY_MINUS) {
			dim += 0.1;
		}
		//  PageDown key - decrease dim
		else if (key==GLFW_KEY_EQUAL && dim>1) {
			dim -= 0.1;
		}
		// Spacee - pause
		else if (key==GLFW_KEY_SPACE) {
			src = !src;
			glBufferSubData(GL_UNIFORM_BUFFER,offsetof(MouseUniformBuffer,source),sizeof(int),&src);
		}
		else if (key==GLFW_KEY_0) {
			// glActiveTexture(GL_TEXTURE0);
			glClearTexImage(texture[0], 0, GL_RGBA, GL_FLOAT, NULL);

		}
    }
    else if (action == GLFW_RELEASE) {
        setTo = 0;
    } 
	else return;

	// Set key if it is a char
    if (key < 256) {
        keys[key] = setTo;
        return;
    }

    // Arrow keys
    else if (key == GLFW_KEY_RIGHT)
        ra = setTo;
    else if (key == GLFW_KEY_LEFT)
        la = setTo;
    else if (key == GLFW_KEY_UP)
        ua = setTo;
    else if (key == GLFW_KEY_DOWN)
        da = setTo;

    //  Exit on ESC
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window,1);
    
}

void updateCursorPosition(GLFWwindow* window, double xpos, double ypos) {
	if (mozUbo) updateMouseCoord(xpos,ypos,width,height);
	return;
} 

void handleInputs() {
	return;
}

void mouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		int clickVal = 1;
		glBufferSubData(GL_UNIFORM_BUFFER,offsetof(MouseUniformBuffer,click),sizeof(int),&clickVal);
	}
	else if (action == GLFW_RELEASE) {
		int clickVal = 0;
		glBufferSubData(GL_UNIFORM_BUFFER,offsetof(MouseUniformBuffer,click),sizeof(int),&clickVal);
	}
}

//
// Helper function to create a VAO with static draw for storing an object, using vdata for vbo and idata for ibo
//
GLuint CreateStaticVertexBuffer(int vsize, void* vdata, int isize, void* idata) {
    // Combine as buffer array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Make vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vsize, vdata, GL_STATIC_DRAW);

    // Make Index buffer object
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, idata, GL_STATIC_DRAW);

    // Return name of vao
    return vao;
}

// Create a vao object for rendering
int CreateObject(int shader, int vsize, void* vdata, int isize, void* idata) {
	GLuint name = CreateStaticVertexBuffer(vsize,vdata,isize,idata);

	glUseProgram(shader);
	// Vertex Attributes
    // Get locations of attributes in shader
    int posLoc = glGetAttribLocation(shader,"aPos");
    int texLoc = glGetAttribLocation(shader,"aTex");

    // Enable VAOs
    if (posLoc != -1) glEnableVertexAttribArray(posLoc);
    if (texLoc != -1) glEnableVertexAttribArray(texLoc);

    // Set vertex attribute pointers
    if (posLoc != -1) glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Pos));
    if (texLoc != -1) glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Tex));

	return name;
}


int main(int argc, char** argv) {
	// Init
	window = InitWindow("Drew Smith", 0, width, height , reshape, key);
	// glfwSetCursorPosCallback(window, updateCursorPosition);
	glfwSetMouseButtonCallback(window, mouseClickCallback);

	// Shaders
	shader[0] = CreateShaderProg("fluid.vert","fluid.frag");
	shader[1] = CreateComputeProg("densitySolver.comp");
	shader[2] = CreateComputeProg("velocitySolver.comp");

	// Create UBO
	glGenBuffers(1,&mozUbo);
	glBindBuffer(GL_UNIFORM_BUFFER,mozUbo);
	struct MouseUniformBuffer mubo = {
		.mozCoord={mx,my},
		.mozVel={0},
		.deltaTime=0,
		.click = 0,
		.source = 0,
	};
	glBufferData(GL_UNIFORM_BUFFER,sizeof(MouseUniformBuffer),&mubo,GL_DYNAMIC_DRAW);
	// Bind UBO to binding point 
	GLuint bindingPoint = 0;
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, mozUbo);
	// Find the uniform block in the shader and bind to the binding point
	GLuint blockIndex = glGetUniformBlockIndex(shader[0], "MouseUniformBuffer");
	glUniformBlockBinding(shader[0], blockIndex, bindingPoint);

	// Create Textures
	glGenTextures(2, texture);

	// Divergence texture
	glActiveTexture(GL_TEXTURE1); // Activate texture unit 1
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Allocate size
	float dat2[512*512*1] = {1};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, dat2);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Bind to texture 1
	glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	ErrCheck("Texture 2");

	// Velocity, density texture
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Allocate size
	float dat[512*512*4] = {0};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Bind to texture 0
	glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	ErrCheck("Texture 1");

	// Set the textures to the correct binding
	// glBindTexture(GL_TEXTURE_2D,texture[0]);
	glUseProgram(shader[0]);
	GLuint loc = glGetUniformLocation(shader[0], "FluidTex");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(shader[0], "DivTex");
	glUniform1i(loc, 1);
	
	glUseProgram(shader[1]);
	loc = glGetUniformLocation(shader[1], "FluidTex");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(shader[1], "DivTex");
	glUniform1i(loc, 1);
	ErrCheck("Texture Uniforms");

	// Objects
	objects[0] = CreateObject(shader[0],sizeof(SurfaceVertices),SurfaceVertices,sizeof(SurfIndices),SurfIndices); // Draw surface
	ErrCheck("init");

	// Main loop
	while(!glfwWindowShouldClose(window)) {
		handleInputs();
		display(window);
		glfwPollEvents();
		updateTime();
	}

	// Exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
