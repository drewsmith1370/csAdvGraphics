#include "CSCIx239.h"
#include "objtovao.c"

#define NUM_FILTERS 3

// Inputs
char keys[256] = {0};
int ua = 0, da = 0, la = 0, ra = 0;
// View
float th = 180;
float ph = 30;
float fov = 59;
float dim = 2;
float asp = 1;
// Misc
GLFWwindow* window=NULL;
int shader[1] = {0};
int filter[NUM_FILTERS] = {0};
char* filterNames[] = {"SSAO","Depth","None"};
int blurFilter = 0;
int useBlur = 0;
float lightPosition[3] = {0,1.5,-3};
int filterMode = 0;
unsigned int fbuf[2]={0};
unsigned int ftex[2]={0};
unsigned int zbuf=0;
unsigned int ztex=0;
// Matrices
unsigned int lightLoc = 0;
// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
int paused = 0;
// Objects
int lighting = 1;
unsigned int objects[3] = {0};
unsigned int nonlocal = 0;
int objIndices = 0;

typedef struct Vertex {
	float Pos[3];
	float Nrm[3];
	float Tex[2];
	float Col[3];
} Vertex;

struct Vertex CubeVertices[] = {
	// Front
	{.Pos={-1,-1, 1}, .Nrm={ 0, 0, 1}, .Tex={0,0}, .Col={ 1, 0, 0}}, //  0
	{.Pos={-1, 1, 1}, .Nrm={ 0, 0, 1}, .Tex={0,1}, .Col={ 1, 0, 0}}, //  1
	{.Pos={ 1, 1, 1}, .Nrm={ 0, 0, 1}, .Tex={1,1}, .Col={ 1, 0, 0}}, //  2
	{.Pos={ 1,-1, 1}, .Nrm={ 0, 0, 1}, .Tex={1,0}, .Col={ 1, 0, 0}}, //  3

	// Left
	{.Pos={-1,-1,-1}, .Nrm={-1, 0, 0}, .Tex={0,0}, .Col={ 0, 1, 0}}, //  4
	{.Pos={-1, 1,-1}, .Nrm={-1, 0, 0}, .Tex={0,1}, .Col={ 0, 1, 0}}, //  5
	{.Pos={-1, 1, 1}, .Nrm={-1, 0, 0}, .Tex={1,1}, .Col={ 0, 1, 0}}, //  6
	{.Pos={-1,-1, 1}, .Nrm={-1, 0, 0}, .Tex={1,0}, .Col={ 0, 1, 0}}, //  7

	// Right
	{.Pos={ 1,-1,-1}, .Nrm={ 1, 0, 0}, .Tex={0,0}, .Col={ 0, 0, 1}}, //  8
	{.Pos={ 1, 1,-1}, .Nrm={ 1, 0, 0}, .Tex={0,1}, .Col={ 0, 0, 1}}, //  9
	{.Pos={ 1, 1, 1}, .Nrm={ 1, 0, 0}, .Tex={1,1}, .Col={ 0, 0, 1}}, // 10
	{.Pos={ 1,-1, 1}, .Nrm={ 1, 0, 0}, .Tex={1,0}, .Col={ 0, 0, 1}}, // 11

	// Back
	{.Pos={-1,-1,-1}, .Nrm={ 0, 0,-1}, .Tex={0,0}, .Col={ 1, 1, 0}}, // 12
	{.Pos={-1, 1,-1}, .Nrm={ 0, 0,-1}, .Tex={0,1}, .Col={ 1, 1, 0}}, // 13
	{.Pos={ 1, 1,-1}, .Nrm={ 0, 0,-1}, .Tex={1,1}, .Col={ 1, 1, 0}}, // 14
	{.Pos={ 1,-1,-1}, .Nrm={ 0, 0,-1}, .Tex={1,0}, .Col={ 1, 1, 0}}, // 15

	// Top
	{.Pos={-1, 1, 1}, .Nrm={ 0, 1, 0}, .Tex={0,0}, .Col={ 0, 1, 1}}, // 16
	{.Pos={-1, 1,-1}, .Nrm={ 0, 1, 0}, .Tex={0,1}, .Col={ 0, 1, 1}}, // 17
	{.Pos={ 1, 1,-1}, .Nrm={ 0, 1, 0}, .Tex={1,1}, .Col={ 0, 1, 1}}, // 18
	{.Pos={ 1, 1, 1}, .Nrm={ 0, 1, 0}, .Tex={1,0}, .Col={ 0, 1, 1}}, // 19

	// Bottom
	{.Pos={-1,-1, 1}, .Nrm={ 0,-1, 0}, .Tex={0,0}, .Col={ 1, 0, 1}}, // 20
	{.Pos={-1,-1,-1}, .Nrm={ 0,-1, 0}, .Tex={0,1}, .Col={ 1, 0, 1}}, // 21
	{.Pos={ 1,-1,-1}, .Nrm={ 0,-1, 0}, .Tex={1,1}, .Col={ 1, 0, 1}}, // 22
	{.Pos={ 1,-1, 1}, .Nrm={ 0,-1, 0}, .Tex={1,0}, .Col={ 1, 0, 1}}, // 23
};

GLuint CubeIndices[] = {
	// Front
	0,1,3 , 3,1,2,
	// Left
	4,5,7 , 7,5,6,
	// Right
	8,9,11 , 11,9,10,
	// Back
	12,13,15 , 15,13,14,
	// Top
	16,17,19 , 19,17,18,
	// Bottom
	20,21,23 , 23,21,22
};

struct Vertex SurfaceVertices[] = {
	{.Pos={-10,0, 10}, .Nrm={ 0, 1, 0}, .Tex={0,0}, .Col={ 0,.5, 1}}, // 20
	{.Pos={-10,0,-10}, .Nrm={ 0, 1, 0}, .Tex={0,1}, .Col={ 0,.5, 1}}, // 21
	{.Pos={ 10,0,-10}, .Nrm={ 0, 1, 0}, .Tex={1,1}, .Col={ 0,.5, 1}}, // 22
	{.Pos={ 10,0, 10}, .Nrm={ 0, 1, 0}, .Tex={1,0}, .Col={ 0,.5, 1}}
};

GLuint SurfIndices[] = {
	0,1,3 , 3,1,2
};

void DisplayObject(unsigned int vao, int n, int texture) {
	glBindTexture(GL_TEXTURE_2D,texture);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES,n,GL_UNSIGNED_INT,0);
	ErrCheck("object");
}

void useFilter() {
	//  Enable shader
	glUseProgram(blurFilter);
	//  Set screen resolution uniforms
	int id,width,height;
	glfwGetWindowSize(window,&width,&height);
	id = glGetUniformLocation(blurFilter,"dx");
	glUniform1f(id,1.0/width);
	id = glGetUniformLocation(blurFilter,"dy");
	glUniform1f(id,1.0/height);

	//  Enable shader
	glUseProgram(filter[filterMode]);
	//  Set screen resolution uniforms
	id = glGetUniformLocation(filter[filterMode],"dx");
	glUniform1f(id,1.0/width);
	id = glGetUniformLocation(filter[filterMode],"dy");
	glUniform1f(id,1.0/height);
	//  Identity projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//  Disable depth test & Enable textures
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	//  Copy entire screen
	int N=10;
	for (int i=0;i<N;i++)
	{
		if (i==0) glUseProgram(filter[filterMode]);
		else if (useBlur) glUseProgram(blurFilter);
		else glUseProgram(filter[2]); // pass through
		//  Output to alternate framebuffers
		//  Final output is to screen
		glBindFramebuffer(GL_FRAMEBUFFER,i==N-1?0:fbuf[(i+1)%2]);
		//  Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);
		//  Input image is from the last framebuffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,ftex[i%2]);
		// Z buffer texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,ztex);
		//  Redraw the screen
		glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(-1,-1);
		glTexCoord2f(0,1); glVertex2f(-1,+1);
		glTexCoord2f(1,1); glVertex2f(+1,+1);
		glTexCoord2f(1,0); glVertex2f(+1,-1);
		glEnd();
	}
	//  Disable textures and shaders
	glDisable(GL_TEXTURE_2D);
	// glUseProgram(0);
	ErrCheck("filter");
}

// Render and swap buffers
void display(GLFWwindow* window) {
	glBindFramebuffer(GL_FRAMEBUFFER,fbuf[0]);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	View(th,ph,fov,dim);

	lightPosition[0] = 3*Cos(progTime*100);
	lightPosition[1] = 1.5;
	lightPosition[2] = 3*Sin(progTime*100);

	if (lighting) {
		Lighting(lightPosition[0],lightPosition[1],lightPosition[2],0,0,0);
		//  Enable shader
		glUseProgram(shader[0]);
	}
	else {
		glDisable(GL_LIGHT0);
		glUseProgram(shader[1]);
	}
	
	//  Draw scene
	// Tyra
	glPushMatrix();
	// glTranslatef(3,0,3);
	glRotated(90,0,1,0);
	DisplayObject(objects[0],objIndices,0);
	glPopMatrix();

	// Cube
	glPushMatrix();
	glTranslatef(-3,0,3);
	glRotated(45,0,1,0);
	DisplayObject(objects[1],35,0); 
	glPopMatrix();

	// Floor
	glPushMatrix();
	glTranslatef(0,-1.3,0);
	DisplayObject(objects[2],6,0); 
	glPopMatrix();

	ErrCheck("scene");

	useFilter();

	//  Revert to fixed pipeline
	glUseProgram(shader[0]);
	glDisable(GL_LIGHTING);
	//  Display parameters
	glWindowPos2i(5,5);
	Print("FPS:%d Filter:%s Blur:%s Lighting:%s",FramesPerSecond(),filterNames[filterMode], useBlur?"on":"off", lighting?"on":"off");
	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	glfwSwapBuffers(window);
}

// Reshape callback
void reshape(GLFWwindow* window, int width, int height) {
	//  Get framebuffer dimensions (makes Apple work right)
	glfwGetFramebufferSize(window,&width,&height);
	//  Ratio of the width to the height of the window
	asp = (height>0) ? (double)width/height : 1;
	//  Set the viewport to the entire window
	glViewport(0,0, width,height);
	//  Set projection
	Projection(fov,asp,dim);

	//
	//  Allocate a frame buffer
	//  Typically the same size as the screen (W,H) but can be larger or smaller
	//
	//  Delete old frame buffer, depth buffer and texture
	if (ztex)
	{
		glDeleteRenderbuffers(1,&zbuf);
		glDeleteTextures(2,ftex);
		glDeleteFramebuffers(2,fbuf);
	}
	//  Allocate two textures, two frame buffer objects and a depth buffer
	glGenFramebuffers(2,fbuf);   
	glGenTextures(2,ftex);
	// glGenRenderbuffers(1,&zbuf);   
	glGenTextures(1,&ztex);
	//  Allocate and size texture
	for (int k=0;k<2;k++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,ftex[k]);
		glTexImage2D(GL_TEXTURE_2D,0,3,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		//  Bind frame buffer to texture
		glBindFramebuffer(GL_FRAMEBUFFER,fbuf[k]);
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,ftex[k],0);
		//  Bind depth buffer to frame buffer 0
		if (k==0)
		{

			// // Bind framebuffer
			// glBindRenderbuffer(GL_RENDERBUFFER,zbuf);
			// glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width,height);
			// glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,zbuf);
			// Create the depth texture
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,ztex);
			glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT,width,height, 0,GL_DEPTH_COMPONENT, GL_FLOAT,NULL);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
			// Bind the framebuffer to the renderbuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,ztex,0);

      	}
   	}
	//  Switch back to regular display buffer
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	ErrCheck("Framebuffer");
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
}

// Handle new keys from glfw
void key(GLFWwindow* window,int key,int scancode,int action,int mods) {
    char setTo;
	// Figure out if key was pressed or released
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        setTo = 1;

		//  Reset view angle
		if (key==GLFW_KEY_0)
			th = ph = 0;
		//  Switch shaders
		else if (key==GLFW_KEY_M) {
			filterMode = (filterMode + 1) % NUM_FILTERS;
		}
		//  PageUp key - increase dim
		else if (key==GLFW_KEY_MINUS) {
			dim += 0.1;
		}
		//  PageDown key - decrease dim
		else if (key==GLFW_KEY_EQUAL && dim>1) {
			dim -= 0.1;
		}
		// Spacee - pause
		else if (key==GLFW_KEY_SPACE && dim>1) {
			paused = !paused;
		}
		// l - lighting
		else if (key==GLFW_KEY_L) {
			lighting = !lighting;
		}
		else if (key==GLFW_KEY_B) {
			useBlur = !useBlur;
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

void handleInputs() {
	// Special
    if (la)
        th -= 75 * deltaTime;
    if (ra) 
        th += 75 * deltaTime;
    if (ua) 
        ph -= 75 * deltaTime;
    if (da) 
        ph += 75 * deltaTime;

    if (th > 360) th -= 360;
	if (th <   0) th += 360;
	if (ph >  90) ph =  90;
	if (ph < -90) ph = -90;
   //  Update projection
   Projection(fov,asp,dim);
}

int CreateObject(int shader, int vsize, void* vdata, int isize, void* idata) {
	GLuint name = CreateStaticVertexBuffer(vsize,vdata,isize,idata);

	glUseProgram(shader);
	// Vertex Attributes
    // Get locations of attributes in shader
    int posLoc = glGetAttribLocation(shader,"Pos");
    int nrmLoc = glGetAttribLocation(shader,"Nrm");
    int texLoc = glGetAttribLocation(shader,"Tex");
    int colLoc = glGetAttribLocation(shader,"Col");

    // Enable VAOs
    if (posLoc != -1) glEnableVertexAttribArray(posLoc);
    if (nrmLoc != -1) glEnableVertexAttribArray(nrmLoc);
    if (texLoc != -1) glEnableVertexAttribArray(texLoc);
    if (colLoc != -1) glEnableVertexAttribArray(colLoc);

    // Set vertex attribute pointers
    if (posLoc != -1) glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Pos));
    if (nrmLoc != -1) glVertexAttribPointer(nrmLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Nrm));
    if (texLoc != -1) glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Tex));
    if (colLoc != -1) glVertexAttribPointer(colLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Col));

	return name;
}


int main(int argc, char** argv) {
	// Init
	window = InitWindow("Drew Smith", 0, 800,600 , reshape, key);

	// Shaders
	shader[0] = CreateShaderProg("light.vert","light.frag");
	shader[1] = CreateShaderProg("light.vert","nolight.frag");
	filter[0] = CreateShaderProg(NULL,"ssao.frag");
	filter[1] = CreateShaderProg(NULL,"depth.frag");
	filter[2] = CreateShaderProg(NULL,"pass.frag");
	blurFilter= CreateShaderProg(NULL,"blur.frag");
	// Objects
	objIndices = ObjToVao(&objects[0],"tyra.obj",shader[0]); // Tyra
	objects[1] = CreateObject(shader[0],sizeof(CubeVertices),CubeVertices,sizeof(CubeIndices),CubeIndices); // Cube
	objects[2] = CreateObject(shader[0],sizeof(SurfaceVertices),SurfaceVertices,sizeof(SurfIndices),SurfIndices); // Floor
	ErrCheck("init");

	glUseProgram(filter[0]);
	GLuint uniLoc;
	uniLoc = glGetUniformLocation(filter[0],"img");
	glUniform1i(uniLoc,0);
	uniLoc = glGetUniformLocation(filter[0],"zbuf");
	glUniform1i(uniLoc,1);

	glUseProgram(filter[1]);
	uniLoc = glGetUniformLocation(filter[0],"img");
	glUniform1i(uniLoc,0);
	uniLoc = glGetUniformLocation(filter[1],"zbuf");
	glUniform1i(uniLoc,1);

	glUseProgram(blurFilter);
	uniLoc = glGetUniformLocation(blurFilter,"img");
	glUniform1i(uniLoc,0);

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
