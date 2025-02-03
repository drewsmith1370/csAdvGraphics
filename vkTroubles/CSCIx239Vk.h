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

#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))

#define NFRAMES 2                                           // Frames in flight
#define RGBA_FORMAT VK_FORMAT_R8G8B8A8_SRGB                 // RGBA format

#ifdef __cplusplus
extern "C" {
#endif



// Helper Functions
// Print message to stderr and exit
void Fatal(const char* format , ...);
//  Initialize GLFW and create window
GLFWwindow* InitWindow(const char* title,int sync,int width,int height , void (*reshape)(GLFWwindow*,int,int) , void (*key)(GLFWwindow*,int,int,int,int));
// Print error enumerant
const char* ErrString(VkResult err);
//  Find index if memory type
uint32_t findMemoryType(uint32_t typeFilter,VkMemoryPropertyFlags properties);
//  Create a buffer and allocate memory
void CreateBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties,VkBuffer* buffer,VkDeviceMemory* bufferMemory);
//  Copy buffer
void CopyBuffer(VkBuffer srcBuffer,VkBuffer dstBuffer,VkDeviceSize size);

//  Wait for previous operations to complete
VkResult WaitForFences();
//  Clear in flight fences
void ClearFences();
// Update the swapchain
uint32_t UpdateSwapchain(GLFWwindow* window);
//  Copy uniform data to buffer
void UniformBufferData(int currentFrame, const void* src, size_t n);
// Return aspect ratio based on current swapchain extent
float GetAspectRatio();
//  Flush command buffer and recreate
void RecreateCommandBuffer(uint32_t imageIndex);
// Set semaphores to handle GPU ordering
void QueueForPresentation(GLFWwindow* window, uint32_t imageIndex, int framebufferResized, int* _currentFrame);

//  Define handy Vulkan structs
typedef struct {float x,y;}     vec2;
typedef struct {float x,y,z;}   vec3;
typedef struct {float x,y,z,w;} vec4;
typedef struct {vec3 xyz,nml,rgb;vec2 st;} Vertex;
typedef struct
{
   float model[16],view[16],proj[16],norm[16]; // Transformation matrices
   vec4  pos,Ca,Cd,Cs;                         // Light properties
   vec4  Ks;                                   // Material properties
   float Ns;                                   //
} UniformBufferObject;


//  Create vertex buffer
void CreateVertexBuffer(); // TODO: Generalize this
//  Start one time commands
VkCommandBuffer BeginSingleTimeCommands();
//  End one time commands
void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
//  Transition image layout
void TransitionImageLayout(VkImage image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout);
//  Create image and allocate memory
void CreateImage(uint32_t width, uint32_t height,VkFormat format,VkImageTiling tiling,VkImageUsageFlags usage,VkMemoryPropertyFlags properties,VkImage* image,VkDeviceMemory* imageMemory);
//  Create image view
VkImageView CreateImageView(VkImage image,VkFormat format,VkImageAspectFlags aspectFlags);
//  Create command pool, buffer and semaphores
void CreateCommandSync();
//  Record command buffer
void RecordCommandBuffer(VkCommandBuffer commandBuffer,uint32_t imageIndex);
//  Create Vulkan instance, physical and logical devices
void CreateDevice(GLFWwindow* window, VkPhysicalDevice* _physDevice, VkDevice* _device);
//  Destroy swapchain
void DestroySwapChain();
//  Create swapchain
void CreateSwapChain(int init, GLFWwindow* window);
//  Close vulkan
void DestroyVulkan();

// createShader.c

//  Create shader module
VkShaderModule CreateShaderModule(VkDevice device, const char* file);

// createPipeline.c

//  Create graphics pipeline
VkPipeline CreateGraphicsPipeline();
// Bind the descriptor set for the current frame to the command buffer
void BindDescriptorSets(VkCommandBuffer commandBuffer, int currentFrame);
// Destroy images
void DestroyImages(VkDevice device);
//  Load texture from BMP file
void CreateTextureImage(const char* file, VkDevice device, VkPhysicalDevice physicalDevice);


#include "mat4.h"

#ifdef __cplusplus
}
#endif

#endif