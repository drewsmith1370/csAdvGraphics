#include "CSCIx239.h"
// Inputs
char keys[256] = {0};
int ua = 0, da = 0, la = 0, ra = 0;
// View
float th = 0;
float ph = 0;
float fov = 59;
float dim = 2;
float asp = 1;
// Misc
int mode = 0;
int tex = 0;
int obj = 0;
int shader = 0;
const char* text[] = {"Fixed Pipeline", "NDC Shader"};

// Render and swap buffers
void display(GLFWwindow* window) {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	View(th,ph,fov,dim);

	//  Enable shader
	if (mode)
	{
		glUseProgram(shader);
		int id = glGetUniformLocation(shader,"time");
		glUniform1f(id,glfwGetTime());
	}
	else
		glUseProgram(0);
	//  Draw scene
	if (obj)
		TexturedIcosahedron(tex);
	else
		TexturedCube(tex);
	//  Revert to fixed pipeline
	glUseProgram(0);

	//  Display axes
	Axes(2);
	//  Display parameters
	SetColor(1,1,1);
	glWindowPos2i(5,5);
	Print("Angle=%.1f,%.1f  Dim=%.1f Projection=%s Mode=%s",th,ph,dim,fov>0?"Perpective":"Orthogonal",text[mode]);
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

// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
// Update time variables each frame
void updateTime() {
    // Update time and deltaTime
    double now = glfwGetTime();
    deltaTime = now - prevTime;
	// Increment time
	progTime += deltaTime;
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
			mode = 1-mode;
		//  Switch objects
		else if (key==GLFW_KEY_O)
			obj = 1-obj;
		//  Switch between perspective/orthogonal
		else if (key==GLFW_KEY_P)
			fov = fov ? 0 : 57;
		//  PageUp key - increase dim
		else if (key==GLFW_KEY_MINUS) {
			dim += 0.1;
		}
		//  PageDown key - decrease dim
		else if (key==GLFW_KEY_EQUAL && dim>1) {
			dim -= 0.1;
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

int main(int argc, char** argv) {
	// Init
	GLFWwindow* window = InitWindow("Drew Smith", 0, 800,600 , reshape, key);

	shader = CreateShaderProg("ndc.vert","ndc.frag");
	tex = LoadTexBMP("pi.bmp");

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
