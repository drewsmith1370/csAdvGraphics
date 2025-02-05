#include "CSCIx239Vk.h"

#ifndef VKDEVICE
#define VKDEVICE

typedef struct Device
{
	// Physical device representation
	VkPhysicalDevice physicalDevice;
	// Logical device representation (application's view of the device)
	VkDevice logicalDevice;
	// Properties of the physical device including limits that the application can check against
	VkPhysicalDeviceProperties properties;
	// Features of the physical device that an application can use to check if a feature is supported
	VkPhysicalDeviceFeatures features;
	// Features that have been enabled for use on the physical device
	VkPhysicalDeviceFeatures enabledFeatures;
	// Memory types and heaps of the physical device
	VkPhysicalDeviceMemoryProperties memoryProperties;
	// Queue family properties of the physical device
    uint32_t queueFamilyCount;
	VkQueueFamilyProperties* queueFamilyProperties;
	// List of extensions supported by the device
    uint32_t extensionCount;
	VkExtensionProperties* supportedExtensions;
	// Default command pool for the graphics queue family index
	VkCommandPool commandPool;
	// Contains queue family indices
	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;
};

// Create Device
struct Device CreateDevice();
// Destroy device
void DestroyDevice();
// Helper functions
uint32_t        GetDeviceMemoryType           (struct Device device, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound);
uint32_t        GetDeviceQueueFamilyIndex     (struct Device device, VkQueueFlags queueFlags);
VkResult        CreateLogicalDevice           (struct Device* device, VkPhysicalDeviceFeatures enabledFeatures, char** enabledExtensions, int numExtensions, void *pNextChain, int useSwapChain, VkQueueFlags requestedQueueTypes);
VkResult        DeviceCreateBuffer            (struct Device  device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, struct Buffer *buffer, VkDeviceSize size, void *data);
void            DeviceCopyBuffer              (struct Device* device, struct Buffer *src, struct Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion);
VkCommandPool   CreateCommandPool             (struct Device* device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);
VkCommandBuffer CreateCommandBuffer           (struct Device* device, VkCommandBufferLevel level, VkCommandPool pool, int begin);
void            FlushCommandBuffer            (struct Device* device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, int free);
int             ExtensionSupported            (struct Device  device, char* extension);
VkFormat        GetSupportedDepthFormat       (struct Device  device, int checkSamplingSupport);

#endif