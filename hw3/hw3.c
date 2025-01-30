#include "CSCIx239.h"
#include "objtovao.c"

// Inputs
char keys[256] = {0};
int ua = 0, da = 0, la = 0, ra = 0;
// View
float th = 30;
float ph = 30;
float fov = 59;
float dim = 2;
float asp = 1;
// Misc
int shader[1] = {0};
float lightPosition[3] = {0,1.5,-3};
char* text[4] = {"Localized VAO","Nonlocal VAO","Display Lists","Cube VAO"};
// Matrices
unsigned int lightLoc = 0;
// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
int paused = 0;
// Objects
int loaderMode = 0;
unsigned int objects[2] = {0};
unsigned int nonlocal = 0;
int objIndices = 0;
int lists[1] = {0};

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
	{.Pos={-5,0, 5}, .Nrm={ 0, 1, 0}, .Tex={0,0}, .Col={ 0,.5, 1}}, // 20
	{.Pos={-5,0,-5}, .Nrm={ 0, 1, 0}, .Tex={0,1}, .Col={ 0,.5, 1}}, // 21
	{.Pos={ 5,0,-5}, .Nrm={ 0, 1, 0}, .Tex={1,1}, .Col={ 0,.5, 1}}, // 22
	{.Pos={ 5,0, 5}, .Nrm={ 0, 1, 0}, .Tex={1,0}, .Col={ 0,.5, 1}}
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

// Render and swap buffers
void display(GLFWwindow* window) {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	View(th,ph,fov,dim);

	lightPosition[0] = 3*Cos(progTime*100);
	lightPosition[1] = 1.5;
	lightPosition[2] = 3*Sin(progTime*100);

	Lighting(lightPosition[0],lightPosition[1],lightPosition[2],0,0,0);

	//  Enable shader
	glUseProgram(shader[0]);
	
	//  Draw scene
	if (loaderMode == 3) {
		DisplayObject(objects[1],35,0);
	}
	else if (loaderMode == 2) 
	{
		glUseProgram(shader[1]);
		glCallList(lists[0]);
	}
	else if (loaderMode == 1) {
		DisplayObject(nonlocal,objIndices,0);
	}
	else
	{
		DisplayObject(objects[0],objIndices,0);
	}
	//  Revert to fixed pipeline
	glUseProgram(0);
	glDisable(GL_LIGHTING);

	//  Display axes
	Axes(2);
	//  Display parameters
	SetColor(1,1,1);
	glWindowPos2i(5,5);
	Print("FPS:%d Mode:%s",FramesPerSecond(),text[loaderMode]);
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
			loaderMode++;
			loaderMode%=4;
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
	GLFWwindow* window = InitWindow("Drew Smith", 0, 800,600 , reshape, key);

	// Shaders
	shader[0] = CreateShaderProg("light.vert","light.frag");
	shader[1] = CreateShaderProg("pixel.vert","phong.frag");
	// Objects
	lists[0] = LoadOBJ("tyra.obj");
	objects[1] = CreateObject(shader[0],sizeof(CubeVertices),CubeVertices,sizeof(CubeIndices),CubeIndices);
	objIndices = ObjToVao(&objects[0],"tyra.obj",shader[0]);
	ObjToVaoNonlocal(&nonlocal,"tyra.obj",shader[0]);
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
