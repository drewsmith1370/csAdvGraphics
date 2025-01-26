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
int mode = 0;
int tex = 0;
int obj = 0;
int shader[1] = {0};
int timeUniform[2]= {0};
const char* text[] = {"Water Normals","Ripple", "Base Lighting"};
float lightPosition[3] = {0,1.5,-3};
// Matrices
unsigned int lightLoc = 0;
// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
int paused = 0;
int moveLight=0;
// Objects
unsigned int objects[2] = {0};
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
	// glDrawArrays(GL_TRIANGLES,0,n);
	ErrCheck("object");
}

// Render and swap buffers
void display(GLFWwindow* window) {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	View(th,ph,fov,dim);

	if (moveLight) {
		lightPosition[0] = 3*Cos(progTime*100);
		lightPosition[1] = 1.5;
		lightPosition[2] = 3*Sin(progTime*100);
	}
	Lighting(lightPosition[0],lightPosition[1],lightPosition[2],0,0,0);

	//  Enable shader
	glUseProgram(shader[0]);
	
	//  Draw scene
	if (obj)
		DisplayObject(objects[1],6,tex);
	else {
		DisplayObject(objects[0],36,tex);
	}
	//  Revert to fixed pipeline
	glUseProgram(0);
	glDisable(GL_LIGHTING);

	//  Display axes
	Axes(2);
	//  Display parameters
	SetColor(1,1,1);

	glPointSize(5);
	glBegin(GL_POINTS);
	glVertex3fv(lightPosition);
	glEnd();

	glWindowPos2i(5,5);
	Print("FPS:%d",FramesPerSecond());
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
		else if (key==GLFW_KEY_M)
			mode = (mode+1)%3;
		//  Switch objects
		else if (key==GLFW_KEY_O)
			obj = 1-obj;
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
		// l - move light
		else if (key==GLFW_KEY_L) {
			moveLight = !moveLight;
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

int CreateObject(int shader, int vsize, void* vdata, int isize, void* idata) {
	GLuint name = CreateStaticVertexBuffer(vsize,vdata,isize,idata);

	glUseProgram(shader);
	// Vertex Attributes
    // Get locations of attributes in shader
    int posLoc = glGetAttribLocation(shader,"Pos");
    int nrmLoc = glGetAttribLocation(shader,"Nrm");
    // int texLoc = glGetAttribLocation(shader,"Tex");
    int colLoc = glGetAttribLocation(shader,"Col");

    // Enable VAOs
    glEnableVertexAttribArray(posLoc);
    glEnableVertexAttribArray(nrmLoc);
    // glEnableVertexAttribArray(texLoc);
    glEnableVertexAttribArray(colLoc);

    // Set vertex attribute pointers
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Pos));
    glVertexAttribPointer(nrmLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Nrm));
    // glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Tex));
    glVertexAttribPointer(colLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Col));

	return name;
}


int main(int argc, char** argv) {
	// Init
	GLFWwindow* window = InitWindow("Drew Smith", 0, 800,600 , reshape, key);

	shader[0] = CreateShaderProg("light.vert","light.frag");
	objects[0] = CreateObject(shader[0],sizeof(CubeVertices),CubeVertices,sizeof(CubeIndices),CubeIndices);
	lists[0] = LoadOBJ("tyra.obj");
	ErrCheck("init");

	ObjToVao("tyra.obj");

	// // Main loop
	// while(!glfwWindowShouldClose(window)) {
	// 	handleInputs();
	// 	display(window);
	// 	glfwPollEvents();
	// 	updateTime();
	// }

	// Exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
