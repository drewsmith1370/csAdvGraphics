// Simple start to a Vulkan library
// Made by Drew Smith, based on Vlakkies code

#ifndef CSCIx239Vk
#define CSCIx239Vk

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// fatal.c
const char* ErrString(VkResult err);
void Fatal(const char* format , ...);
// initwin.c
GLFWwindow* InitWindow(const char* title,int sync,int width,int height , void (*reshape)(GLFWwindow*,int,int) , void (*key)(GLFWwindow*,int,int,int,int));

// Debugging macro
#define VK_CHECK(f) {VkResult res=f;\
   if(res != VK_SUCCESS)\
      Fatal("Error: VkResult is %s\n",ErrString(res));}

// Default timeout for fences
#define DEFAULT_FENCE_TIMEOUT 100000000000

#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))

#define NFRAMES 2                                           // Frames in flight
#define RGBA_FORMAT VK_FORMAT_R8G8B8A8_SRGB                 // RGBA format

#ifdef __cplusplus
extern "C" {
#endif

#include "vkBuffer.h"
#include "vkDevice.h"

#include "mat4.h"

#ifdef __cplusplus
}
#endif

#endif