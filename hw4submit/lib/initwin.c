//  CSCIx239 library
//  Willem A. (Vlakkies) Schreuder
#include "CSCIx239Vk.h"


//
// Print GLFW errors to stderr
//
static void error(int error,const char* text)
{
    fprintf(stderr,"GLFW error %d: %s\n",error,text);
}

//
//  Initialize GLFW, GLEW and launch window
//
GLFWwindow* InitWindow(const char* title,int sync,int width,int height , void (*reshape)(GLFWwindow*,int,int) , void (*key)(GLFWwindow*,int,int,int,int))
{
   //  Initialize GLFW
   if (!glfwInit()) Fatal("Cannot initialize glfw\n");

   // Check for Vulkan support
   if (!glfwVulkanSupported()) Fatal("GLFW reports Vulkan not supported\n");

   //  Error callback
   glfwSetErrorCallback(error);

   //  Set window properties
   glfwWindowHint(GLFW_RESIZABLE,1);     //  Window can be resized
   glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);

   //  Create window and make current
   GLFWwindow* window = glfwCreateWindow(width,height,title, NULL, NULL);
   if (!window) Fatal("Cannot create GLFW window\n");

   #ifdef USEGLEW
   //  Initialize GLEW (if compiled in)
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
   #endif

   //  Set callback for window resize and make initial call
   if (reshape)
   {
      glfwSetFramebufferSizeCallback(window,reshape);
      //  Set initial window size and call reshape
   }

   //  Set callback for keyboard input
   if (key) glfwSetKeyCallback(window,key);

   //  Return window
   return window;
}
