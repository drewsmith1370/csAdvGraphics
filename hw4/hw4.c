/*
 *  Shaders - Vulkan style
 *
 *  Adapted from vulkan-tutorial.com
 *
 *  Key bindings:
 *  p/P        Toggle between orthogonal & perspective projection
 *  m/M        Stop/start light movement
 *  -/+        Move light up/down
 *  [/]        Move light in orbit
 *  arrows     Change view angle
 *  ESC        Exit
 *
 */

//  KILLER FACT:  Most structs have LOTS of fields you may not care about,
//    but must be initialized.  Use the C99 syntax
//       var = {.field=val,.field=val,...,.field=val};
//    to make sure that the rest of the struct is properly initialized.
//    Unspecified fields are initialized to zeroes.
#include "CSCIx239Vk.h"

int                      framebufferResized = 0;            // Resize the scissors and viewport
int                      move=1;                            // Light movement
float                    th=0,ph=0,zh=0;                    // View angles
int                      proj=1;                            // Projection type
float                    fov=55;                            // Field of view
float                    Ylight=2;                          // Light elevation
float                    dim=3.0;                           // Size of world
GLFWwindow*              window;                            // GLFW window

// Inputs
char keys[256] = {0};
int ua = 0, da = 0, la = 0, ra = 0;
// Time
double progTime = 0;
double prevTime = 0;
double deltaTime = 0;
int paused = 0;

//
//  Draw frame
//
void display()
{
   //  Wait for previous operations to complete
   WaitForFences();
   uint32_t imageIndex = UpdateSwapchain(window);
   
   //  Update uniforms
   UniformBufferObject ubo;
   //  Model matrix
   mat4identity(ubo.model);
   //  View matrix
   mat4identity(ubo.view);
   if (proj)
   {
      double Ex = -2*dim*Sin(th)*Cos(-ph);
      double Ey = +2*dim        *Sin(-ph);
      double Ez = +2*dim*Cos(th)*Cos(-ph);
      mat4lookAt(ubo.view,Ex,Ey,Ez , 0,0,0 , 0,Cos(-ph),0);
   }
   else
   {
      mat4rotate(ubo.view,-ph,1,0,0);
      mat4rotate(ubo.view,th,0,1,0);
   }
   //  Projection matrix
   float asp = GetAspectRatio();
   mat4identity(ubo.proj);
   if (proj)
      mat4perspective(ubo.proj,fov,asp,dim/16,16*dim);
   else
      mat4ortho(ubo.proj,-asp*dim,+asp*dim,-dim,+dim,-dim,+dim);
   //  Normal matrix
   float MV[16];
   mat4copy(MV,ubo.view);
   mat4multMatrix(MV,ubo.model);
   mat4normalMatrix(ubo.view,ubo.norm);

   //  Set light position based on time
   if (move) zh = fmod(90*glfwGetTime(),360.0);
   ubo.pos.x = 4*Cos(zh);
   ubo.pos.y = Ylight;
   ubo.pos.z = 4*Sin(zh);
   ubo.pos.w = 1;
   //  Set colors
   vec4 grey  = {0.3,0.3,0.3,1};
   vec4 white = {1,1,1,1};
   ubo.Ca = grey;
   ubo.Cd = white;
   ubo.Cs = white;
   ubo.Ks = white;
   ubo.Ns = 16;

   // Copy uniform data to buffer
   UniformBufferData(&ubo,sizeof(ubo));
   // Clear fences
   ClearFences();

   //  Flush command buffer and recreate
   RecreateCommandBuffer(imageIndex);

   //  Set presentation queue
   QueueForPresentation(window,imageIndex,framebufferResized);
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
         //  Toggle projection type
      else if (key == GLFW_KEY_P)
         proj = 1-proj;
      //  Light movement
      else if (key == GLFW_KEY_M)
         move = !move;
      //  Light elevation
      else if (key == GLFW_KEY_KP_ADD)
         Ylight += 0.1;
      else if (key == GLFW_KEY_KP_SUBTRACT)
         Ylight -= 0.1;
      //  Light position
      else if (key==GLFW_KEY_LEFT_BRACKET)
         zh += 1;
      else if (key==GLFW_KEY_RIGHT_BRACKET)
         zh -= 1;
		//  PageUp key - increase dim
		else if (key==GLFW_KEY_MINUS) {
			dim += 0.1;
		}
		//  PageDown key - decrease dim
		else if (key==GLFW_KEY_EQUAL && dim>1) {
			dim -= 0.1;
		}
		// // Spacee - pause
		// else if (key==GLFW_KEY_SPACE && dim>1) {
		// 	paused = !paused;
		// }
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
   // Projection(fov,asp,dim);
}

//
//  Callback for window resized
//
static void resize(GLFWwindow* window,int width,int height)
{
   framebufferResized = 1;
}

//
//  Main program
//
int main()
{
   //  Initialize GLFW
   window = InitWindow("Drew Smith",0,800,600,resize,key);
   mat4vulkan(1);

   //  Create instance, physical and logical devices
   CreateDevice(window);
   //  Create the swapchain, image and framebuffers
   CreateSwapChain(1,window);
   //  Load texture
   CreateTextureImage("pi.bmp");
   //  Create graphics pipeline
   CreateGraphicsPipeline();
   //  Create command pool, buffer and semaphores
   CreateCommandSync();
   //  Create vertex buffer
   CreateVertexBuffer();

   //  Main loop
   while (!glfwWindowShouldClose(window))
   {
      updateTime();
      handleInputs();
      display();
      glfwPollEvents();
   }
 
   //  Cleanup vulkan and GLFW
   DestroyVulkan();
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}
