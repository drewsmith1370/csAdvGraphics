#include "vkBuffer.h"

//
//  Create a buffer object base
//
struct Buffer CreateBuffer(VkDevice device) {
	return (struct Buffer) {
		.device=device,
		.buffer=VK_NULL_HANDLE,
		.memory=VK_NULL_HANDLE,
		.descriptor={},
		.size=0,
		.mapped=NULL,
		.usageFlags=0,
		.memoryPropertyFlags=0
	};
}

//
//  Map a memory range. 
//  Use VK_WHOLE_SIZE for full size.
//
VkResult MapBuffer(struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(buffer->device, buffer->memory, offset, size, 0, &buffer->mapped);
}

//
//  Unmap a buffer
//
void UnmapBuffer(struct Buffer* buffer)
{
    if (buffer->mapped)
    {
        vkUnmapMemory(buffer->device, buffer->memory);
        buffer->mapped = 0;
    }
}

//
//  Bind a buffer
//
VkResult BindBuffer(struct Buffer* buffer, VkDeviceSize offset)
{
    return vkBindBufferMemory(buffer->device, buffer->buffer, buffer->memory, offset);
}

//
//  Setup the descriptor for a buffer
//  Use VK_WHOLE_SIZE for full buffer size
//
void SetupDescriptor(struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset)
{
	buffer->descriptor.offset = offset;
	buffer->descriptor.buffer = buffer->buffer;
	buffer->descriptor.range = size;
}

//
// Copy a buffer
//
void CopyBuffer(struct Buffer buffer, void* data, VkDeviceSize size)
{
	assert(buffer.mapped);
	memcpy(buffer.mapped, data, size);
}

// 
//  Flush a memory range of the buffer to make it visible to the device
//  Use VK_WHOLE_SIZE for full buffer
//
VkResult FlushBuffer(struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = buffer->memory,
		.offset = offset,
		.size = size
	};
	return vkFlushMappedMemoryRanges(buffer->device, 1, &mappedRange);
}

//
//  Invalidate a memory range of the buffer to make it visible to the host
//
VkResult InvalidateBuffer(struct Buffer* buffer, VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = buffer->memory,
		.offset = offset,
		.size = size
	};
	return vkInvalidateMappedMemoryRanges(buffer->device, 1, &mappedRange);
}

//
//  Destroy buffer and release resources
//
void DestroyBuffer(struct Buffer* buffer)
{
	if (buffer->buffer)
	{
		vkDestroyBuffer(buffer->device, buffer->buffer, NULL);
	}
	if (buffer->memory)
	{
		vkFreeMemory(buffer->device, buffer->memory, NULL);
	}
}