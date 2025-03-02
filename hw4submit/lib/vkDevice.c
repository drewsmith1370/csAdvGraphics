#include "CSCIx239Vk.h"
#include "vkDevice.h"

//
//  Initialize and populate a Vulkan Device
//
struct Device CreateDevice(VkPhysicalDevice physicalDevice) {
    assert(physicalDevice);
    struct Device device = 
    {
        .physicalDevice          = physicalDevice,
        .logicalDevice           = 0,
        .properties              = {},
        .features                = {},
        .enabledFeatures         = {},
        .memoryProperties        = {},
        .queueFamilyCount        = 0,
        .queueFamilyProperties   = 0,
        .extensionCount          = 0,
        .supportedExtensions     = 0,
        .commandPool             = 0,
        .queueFamilyIndices      = {}
    };

    // Get properties
    vkGetPhysicalDeviceProperties(physicalDevice, &device.properties);
    // Get features
    vkGetPhysicalDeviceFeatures(physicalDevice, &device.features);
    // Get memory properties
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &device.memoryProperties);
    // Count queue families and get properties 
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &device.queueFamilyCount, NULL);
    assert(device.queueFamilyCount > 0);
    device.queueFamilyProperties = malloc(device.queueFamilyCount*sizeof(VkQueueFamilyProperties));
    if (device.queueFamilyProperties == NULL) {
        Fatal("Fatal: error malloc'ing %d bytes for queue family properties\n",sizeof(VkQueueFamilyProperties)*device.queueFamilyCount);
    }
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &device.queueFamilyCount, device.queueFamilyProperties);
    // Get list of supported extensions
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device.extensionCount, NULL);
    if (device.extensionCount > 0) {
        device.supportedExtensions = (VkExtensionProperties*)malloc(device.extensionCount*sizeof(VkExtensionProperties));
        if (!device.supportedExtensions) Fatal("Error malloc'ing %d bytes\n", device.extensionCount*sizeof(VkExtensionProperties));
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice,NULL,&device.extensionCount,device.supportedExtensions));
    }

    return device;
}

//
//  Free a logical device
//
void DestroyDevice(struct Device device) {
    if (device.commandPool) {
        vkDestroyCommandPool(device.logicalDevice, device.commandPool, NULL);
    }
    if (device.logicalDevice) {
        vkDestroyDevice(device.logicalDevice, NULL);
    }

    free(device.queueFamilyProperties);
    device.queueFamilyProperties = NULL;
    free(device.supportedExtensions);
    device.supportedExtensions = NULL;
}

/**
* Get the index of a memory type that has all the requested property bits set
*
* @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
* @param properties Bit mask of properties for the memory type to request
* @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
* 
* @return Index of the requested memory type
*
* @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested properties
*/
uint32_t GetDeviceMemoryType(struct Device device, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound)
{
    VkPhysicalDeviceMemoryProperties memProps = device.memoryProperties;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memProps.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = 1;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = 0;
        return 0;
    }
    else
    {
        Fatal("Could not find a matching memory type\n");
        return 0;
    }
}

//
//  Get the index of a queue family that supports the requested queue flags
//  SRS - support VkQueueFlags parameter for requesting multiple flags vs. VkQueueFlagBits for a single flag only
//
uint32_t GetDeviceQueueFamilyIndex(struct Device device, VkQueueFlags queueFlags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
    {
        for (uint32_t i = 0; i < device.queueFamilyCount; i++)
        {
            if ((device.queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((device.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
    {
        for (uint32_t i = 0; i < device.queueFamilyCount; i++)
        {
            if ((device.queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((device.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((device.queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < device.queueFamilyCount; i++)
    {
        if ((device.queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
        {
            return i;
        }
    }

    Fatal("Could not find a matching queue family index\n");
    return 0;
}

//
//  Create the logical device based on the assigned physical device, also gets default queue family indices
// 
//  enabledFeatures Can be used to enable certain features upon device creation
//  pNextChain Optional chain of pointer to extension structures
//  useSwapChain Set to false for headless rendering to omit the swapchain device extensions
//  requestedQueueTypes Bit flags specifying the queue types to be requested from the device  
//
VkResult CreateLogicalDevice(struct Device* device, VkPhysicalDeviceFeatures enabledFeatures, char** enabledExtensions, int numExtensions, void* pNextChain, int useSwapChain, VkQueueFlags requestedQueueTypes)
{			
    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    uint32_t numQueues = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[5] = {0}; // was a vector

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    const float defaultQueuePriority = 0;

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
        device->queueFamilyIndices.graphics = GetDeviceQueueFamilyIndex(*device, VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = device->queueFamilyIndices.graphics,
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority
        };
        queueCreateInfos[numQueues++] = queueInfo;
    }
    else
    {
        device->queueFamilyIndices.graphics = 0;
    }

    // Try to get a dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
        device->queueFamilyIndices.compute = GetDeviceQueueFamilyIndex(*device,VK_QUEUE_COMPUTE_BIT);
        if (device->queueFamilyIndices.compute != device->queueFamilyIndices.graphics)
        {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = device->queueFamilyIndices.compute,
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority
            };
            queueCreateInfos[numQueues++] = queueInfo;
        }
    }
    else
    {
        // Else we use the same queue
        device->queueFamilyIndices.compute = device->queueFamilyIndices.graphics;
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
        device->queueFamilyIndices.transfer = GetDeviceQueueFamilyIndex(*device,VK_QUEUE_TRANSFER_BIT);
        if ((device->queueFamilyIndices.transfer != device->queueFamilyIndices.graphics) && (device->queueFamilyIndices.transfer != device->queueFamilyIndices.compute))
        {
            // If transfer family index differs, we need an additional queue create info for the transfer queue
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = device->queueFamilyIndices.transfer;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos[numQueues++] = queueInfo;
        }
    }
    else
    {
        // Else we use the same queue
        device->queueFamilyIndices.transfer = device->queueFamilyIndices.graphics;
    }

    // Create the logical device representation
    char** deviceExtensions;
    if (useSwapChain)
    {
        // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
        numExtensions++;
        deviceExtensions = (char**)malloc(sizeof(char*)*numExtensions);
        if (!deviceExtensions) Fatal("Failed to mallocate %d bytes",sizeof(char*)*numExtensions);\
        
        // Copy existing strings to the new array
        for (size_t i = 0; i < numExtensions-1; i++) {
            deviceExtensions[i] = strdup(enabledExtensions[i]); // Duplicate each string
            if (!deviceExtensions[i]) Fatal("Failed to duplicate string\n");
        }
        // Add new extension
        deviceExtensions[numExtensions-1] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }
    else {
        deviceExtensions = enabledExtensions;
    }

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = numQueues,
        .pQueueCreateInfos = queueCreateInfos,
        .pEnabledFeatures = &enabledFeatures
    };
    
    // If a pNext(Chain) has been passed, we need to add it to the device creation info
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    if (pNextChain) {
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enabledFeatures;
        physicalDeviceFeatures2.pNext = pNextChain;
        deviceCreateInfo.pEnabledFeatures = NULL;
        deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    }

    if (numExtensions > 0)
    {
        for (int i=0;i<numExtensions;i++)
        {
            if (!ExtensionSupported(*device, deviceExtensions[i])) {
                fprintf(stderr,"Enabled device extension \"%s\" is not present at device level\n",deviceExtensions[i]);
            }
        }

        deviceCreateInfo.enabledExtensionCount = numExtensions;
        deviceCreateInfo.ppEnabledExtensionNames = (const char* const*)deviceExtensions;
    }

    device->enabledFeatures = enabledFeatures;

    VkResult result = vkCreateDevice(device->physicalDevice, &deviceCreateInfo, NULL, &device->logicalDevice);
    if (result != VK_SUCCESS) 
    {
        return result;
    }
    
    // Create a default command pool for graphics command buffers
    device->commandPool = CreateCommandPool(device, device->queueFamilyIndices.graphics,0);

    return result;
}

// /**
// * Create a buffer on the device
// *
// * @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
// * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
// * @param size Size of the buffer in byes
// * @param buffer Pointer to the buffer handle acquired by the function
// * @param memory Pointer to the memory handle acquired by the function
// * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
// *
// * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
// */
// VkResult DeviceCreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
// {
//     // Create the buffer handle
//     VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
//     bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//     VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));

//     // Create the memory backing up the buffer handle
//     VkMemoryRequirements memReqs;
//     VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
//     vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
//     memAlloc.allocationSize = memReqs.size;
//     // Find a memory type index that fits the properties of the buffer
//     memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
//     // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
//     VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
//     if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
//         allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
//         allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
//         memAlloc.pNext = &allocFlagsInfo;
//     }
//     VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory));
        
//     // If a pointer to the buffer data has been passed, map the buffer and copy over the data
//     if (data != nullptr)
//     {
//         void *mapped;
//         VK_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
//         memcpy(mapped, data, size);
//         // If host coherency hasn't been requested, do a manual flush to make writes visible
//         if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
//         {
//             VkMappedMemoryRange mappedRange = vks::initializers::mappedMemoryRange();
//             mappedRange.memory = *memory;
//             mappedRange.offset = 0;
//             mappedRange.size = size;
//             vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
//         }
//         vkUnmapMemory(logicalDevice, *memory);
//     }

//     // Attach the memory to the buffer object
//     VK_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

//     return VK_SUCCESS;
// }

//
// Create a buffer on the device
//
VkResult DeviceCreateBuffer(struct Device device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, struct Buffer *buffer, VkDeviceSize size, void *data)
{
    buffer->device = device.logicalDevice;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.usage = usageFlags,
			.size = size
    };
    VK_CHECK(vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, NULL, &buffer->buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
    };
    vkGetBufferMemoryRequirements(device.logicalDevice, buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = GetDeviceMemoryType(device,memReqs.memoryTypeBits, memoryPropertyFlags,NULL);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo = {};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK(vkAllocateMemory(device.logicalDevice, &memAlloc, NULL, &buffer->memory));

    buffer->alignment = memReqs.alignment;
    buffer->size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != NULL)
    {
        VK_CHECK(MapBuffer(buffer,size,0));
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            FlushBuffer(buffer,size,0);

        UnmapBuffer(buffer);
    }

    // Initialize a default descriptor that covers the whole buffer size
    SetupDescriptor(buffer,size,0);

    // Attach the memory to the buffer object
    return BindBuffer(buffer,0);
}

//
//  Copy buffer data from src to dst using VkCmdCopyBuffer
//
void DeviceCopyBuffer(struct Device* device, struct Buffer *src, struct Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion)
{
    assert(dst->size <= src->size);
    assert(src->buffer);
    VkCommandBuffer copyCmd = CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->commandPool, VK_TRUE);
    VkBufferCopy bufferCopy = {};
    if (copyRegion == NULL)
    {
        bufferCopy.size = src->size;
    }
    else
    {
        bufferCopy = *copyRegion;
    }

    vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

    FlushCommandBuffer(device, copyCmd, queue, device->commandPool, VK_TRUE);
}

/** 
* Create a command pool for allocation command buffers from
* 
* @param queueFamilyIndex Family index of the queue to create the command pool for
* @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
*
* @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
*
* @return A handle to the created command buffer
*/
VkCommandPool CreateCommandPool(struct Device* device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
    VkCommandPoolCreateInfo cmdPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndex,
        .flags = createFlags
    };
    VkCommandPool cmdPool;
    VK_CHECK(vkCreateCommandPool(device->logicalDevice, &cmdPoolInfo, NULL, &cmdPool));
    return cmdPool;
}

/**
* Allocate a command buffer from the command pool
*
* @param level Level of the new command buffer (primary or secondary)
* @param pool Command pool from which the command buffer will be allocated
* @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
*
* @return A handle to the allocated command buffer
*/
VkCommandBuffer CreateCommandBuffer(struct Device* device, VkCommandBufferLevel level, VkCommandPool pool, int begin)
{
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = level,
        .commandBufferCount = 1
    };
    VkCommandBuffer cmdBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device->logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
    // If requested, also start recording for the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }
    return cmdBuffer;
}

//
//  Finish command buffer recording and submit it to a queue
//
void FlushCommandBuffer(struct Device* device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, int free)
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = 0
    };
    VkFence fence;
    VK_CHECK(vkCreateFence(device->logicalDevice, &fenceInfo, NULL, &fence));
    // Submit to the queue
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
    // Wait for the fence to signal that command buffer has finished executing
    VK_CHECK(vkWaitForFences(device->logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
    vkDestroyFence(device->logicalDevice, fence, NULL);
    if (free)
    {
        vkFreeCommandBuffers(device->logicalDevice, pool, 1, &commandBuffer);
    }
}

//
//  Check if an extension is supported by the (physical device)
//
int ExtensionSupported(struct Device device, char* extension)
{
    VkExtensionProperties* available = device.supportedExtensions;
    uint32_t n = device.extensionCount;
    for (uint32_t i=0;i<n;i++) {
        if (!strcmp(extension,available[i].extensionName)) 
            return 1;
    }
    return 0;
}

/**
* Select the best-fit depth format for this device from a list of possible depth (and stencil) formats
*
* @param checkSamplingSupport Check if the format can be sampled from (e.g. for shader reads)
*
* @return The depth format that best fits for the current device
*
* @throw Throws an exception if no depth format fits the requirements
*/
VkFormat GetSupportedDepthFormat(struct Device device, int checkSamplingSupport)
{
    // All depth formats may be optional, so we need to find a suitable depth format to use
    VkFormat depthFormats[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    for (int i=0;i<5;i++)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.physicalDevice, depthFormats[i], &formatProperties);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            if (checkSamplingSupport) {
                if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                    continue;
                }
            }
            return depthFormats[i];
        }
    }
    Fatal("Could not find a matching depth format");
    return 0;
}
