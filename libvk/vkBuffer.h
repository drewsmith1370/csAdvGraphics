#ifndef VKBUFFER
#define VKBUFFER

#include <vulkan.h>

struct Buffer
{
    VkDevice device;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size;
    VkDeviceSize alignment;
    void* mapped;
    /** Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags usageFlags;
    /** Memory property flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags memoryPropertyFlags;
};

// Create the buffer with initial values
struct Buffer  CreateBuffer();
// Helper functions
VkResult       MapBuffer        (struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset);
void           UnmapBuffer      (struct Buffer* buffer);
VkResult       BindBuffer       (struct Buffer* buffer, VkDeviceSize offset);
void           SetupDescriptor  (struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset);
void           CopyBuffer       (struct Buffer  buffer, void* data, VkDeviceSize size);
VkResult       FlushBuffer      (struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset);
VkResult       InvalidateBuffer (struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset);
void           DestroyBuffer    (struct Buffer* buffer);

#endif