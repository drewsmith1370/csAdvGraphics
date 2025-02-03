#include "CSCIx239Vk.h"

VkBuffer                 uniformBuffers[NFRAMES];           // Uniform buffers
VkDeviceMemory           uniformBuffersMemory[NFRAMES];     // Uniform buffer memory
void*                    uniformBuffersMapped[NFRAMES];     // Mapping for uniform buffer
VkDescriptorSetLayout    descriptorSetLayout;               // Descriptor set layout
VkPipelineLayout         pipelineLayout;                    // Pipeline layout
VkDescriptorPool         descriptorPool;                    // Descriptor pool
VkDescriptorSet          descriptorSets[NFRAMES];           // Descriptor sets
VkImage                  textureImage;                      // Texture image
VkDeviceMemory           textureImageMemory;                // Texture image memory
VkImageView              textureImageView;                  // Texture image view
VkSampler                textureSampler;                    // Texture sampler


//
//  Reverse n bytes
//
static void Reverse(void* x,const int n)
{
   char* ch = (char*)x;
   for (int k=0;k<n/2;k++)
   {
      char tmp = ch[k];
      ch[k] = ch[n-1-k];
      ch[n-1-k] = tmp;
   }
}


// Create a uniform buffer
void CreateUniformBuffer(VkDevice device) {
    //  Create uniform buffers
    for (int k=0;k<NFRAMES;k++)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        CreateBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&uniformBuffers[k],&uniformBuffersMemory[k]);
        vkMapMemory(device,uniformBuffersMemory[k],0,bufferSize,0,&uniformBuffersMapped[k]);
    }

    //  Uniform buffer layout
    VkDescriptorSetLayoutBinding uboLayoutBinding =
    {
        .binding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImmutableSamplers = NULL,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding =
    {
        .binding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImmutableSamplers = NULL,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    //  Create descriptor layout
    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = sizeof(bindings)/sizeof(VkDescriptorSetLayoutBinding),
        .pBindings = bindings,
    };
    if (vkCreateDescriptorSetLayout(device,&layoutInfo,NULL,&descriptorSetLayout))
        Fatal("Failed to create descriptor set layout\n");

}

//
//  Load texture from BMP file
//
void CreateTextureImage(const char* file, VkDevice device, VkPhysicalDevice physicalDevice)
{
   //  Open file
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);
   //  Check image magic
   unsigned short magic;
   if (fread(&magic,2,1,f)!=1) Fatal("Cannot read magic from %s\n",file);
   if (magic!=0x4D42 && magic!=0x424D) Fatal("Image magic not BMP in %s\n",file);
   //  Read header
   unsigned int dx,dy,off,k; // Image dimensions, offset and compression
   unsigned short nbp,bpp;   // Planes and bits per pixel
   if (fseek(f,8,SEEK_CUR) || fread(&off,4,1,f)!=1 ||
       fseek(f,4,SEEK_CUR) || fread(&dx,4,1,f)!=1 || fread(&dy,4,1,f)!=1 ||
       fread(&nbp,2,1,f)!=1 || fread(&bpp,2,1,f)!=1 || fread(&k,4,1,f)!=1)
     Fatal("Cannot read header from %s\n",file);
   //  Reverse bytes on big endian hardware (detected by backwards magic)
   if (magic==0x424D)
   {
      Reverse(&off,4);
      Reverse(&dx,4);
      Reverse(&dy,4);
      Reverse(&nbp,2);
      Reverse(&bpp,2);
      Reverse(&k,4);
   }
   //  Get physical device properties
   VkPhysicalDeviceProperties properties;
   vkGetPhysicalDeviceProperties(physicalDevice,&properties);
   unsigned int max=properties.limits.maxImageDimension2D;
   //  Check image parameters
   if (dx<1 || dx>max) Fatal("%s image width %d out of range 1-%d\n",file,dx,max);
   if (dy<1 || dy>max) Fatal("%s image height %d out of range 1-%d\n",file,dy,max);
   if (nbp!=1)  Fatal("%s bit planes is not 1: %d\n",file,nbp);
   if (bpp!=24) Fatal("%s bits per pixel is not 24: %d\n",file,bpp);
   if (k!=0)    Fatal("%s compressed files not supported\n",file);

   //  Allocate image memory
   VkDeviceSize imageSize = 4*dx*dy;
   unsigned char* image = (unsigned char*) malloc(imageSize);
   if (!image) Fatal("Cannot allocate %d bytes of memory for image %s\n",imageSize,file);

   //  Seek to and read image
   if (fseek(f,off,SEEK_SET) || fread(image,3*dx*dy,1,f)!=1) Fatal("Error reading data from image %s\n",file);
   fclose(f);
   //  Map BGR to RGBA (RGBA is better supported)
   for (int i=dx*dy-1;i>=0;i--)
   {
      unsigned char R = image[3*i+2];
      unsigned char G = image[3*i+1];
      unsigned char B = image[3*i+0];
      image[4*i+0] = R;
      image[4*i+1] = G;
      image[4*i+2] = B;
      image[4*i+3] = 255;
   }

   //  Create staging buffer
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   CreateBuffer(imageSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&stagingBuffer,&stagingBufferMemory);
   //  Copy image to buffer
   void* data;
   vkMapMemory(device,stagingBufferMemory,0,imageSize,0,&data);
   memcpy(data,image,imageSize);
   vkUnmapMemory(device,stagingBufferMemory);
   free(image);
   //  Create Image
   CreateImage(dx,dy,RGBA_FORMAT,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,&textureImage,&textureImageMemory);
   //  Transfer buffer to image
   TransitionImageLayout(textureImage,RGBA_FORMAT,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
   VkBufferImageCopy region =
   {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .imageSubresource.mipLevel = 0,
      .imageSubresource.baseArrayLayer = 0,
      .imageSubresource.layerCount = 1,
      .imageOffset = {0,0,0},
      .imageExtent = {dx,dy,1},
   };
   vkCmdCopyBufferToImage(commandBuffer,stagingBuffer,textureImage,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
   EndSingleTimeCommands(commandBuffer);
   TransitionImageLayout(textureImage,RGBA_FORMAT,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

   //  Destroy staging buffer
   vkDestroyBuffer(device,stagingBuffer,NULL);
   vkFreeMemory(device,stagingBufferMemory,NULL);

   //  Create texture sampler
   VkSamplerCreateInfo samplerInfo =
   {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
   };
   if (vkCreateSampler(device,&samplerInfo,NULL,&textureSampler))
      Fatal("Failed to create texture sampler\n");

   //  Create Image View
   textureImageView = CreateImageView(textureImage,RGBA_FORMAT,VK_IMAGE_ASPECT_COLOR_BIT);
}


//  Copy uniform data to buffer
void UniformBufferData(int currentFrame, const void* src, size_t n) {
   memcpy(uniformBuffersMapped[currentFrame],src,n);
}

void CreateDescriptorPool(VkDevice device) {
    //  Create descriptor pool
    VkDescriptorPoolSize poolSizes[] =
    {
        {.type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        , .descriptorCount=NFRAMES},
        {.type=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount=NFRAMES},
    };

    VkDescriptorPoolCreateInfo dsPoolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = sizeof(poolSizes)/sizeof(VkDescriptorPoolSize),
        .pPoolSizes = poolSizes,
        .maxSets = NFRAMES,
    };
    if (vkCreateDescriptorPool(device,&dsPoolInfo,NULL,&descriptorPool))
        Fatal("Failed to create descriptor pool\n");
}

// Create Descriptor sets using descriptor pool
void CreateDescriptorSets(VkDevice device) {

    //  Create descriptor sets
    VkDescriptorSetLayout layouts[NFRAMES];
    for (int k=0;k<NFRAMES;k++)
    layouts[k] =  descriptorSetLayout;
    VkDescriptorSetAllocateInfo dsAllocInfo =
    {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptorPool,
    .descriptorSetCount = NFRAMES,
    .pSetLayouts = layouts,
    };
    if (vkAllocateDescriptorSets(device,&dsAllocInfo,descriptorSets))
    Fatal("Failed to allocate descriptor sets\n");
    //  Set descriptors for each frame
    for (int k=0;k<NFRAMES;k++)
    {
        VkDescriptorBufferInfo bufferInfo =
        {
            .buffer = uniformBuffers[k],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        VkDescriptorImageInfo imageInfo =
        {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = textureImageView,
            .sampler = textureSampler,
        };

        VkWriteDescriptorSet descriptorWrites[] =
        {
            {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[k],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo,
            },
            {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[k],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &imageInfo,
            },
        };

        vkUpdateDescriptorSets(device,sizeof(descriptorWrites)/sizeof(VkWriteDescriptorSet),descriptorWrites,0,NULL);
    }
}

// Bind the descriptor set for the current frame to the command buffer
void BindDescriptorSets(VkCommandBuffer commandBuffer, int currentFrame) {
   vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,&descriptorSets[currentFrame],0,NULL);
}

// 
VkPipeline CreateGraphicsPipeline(VkDevice device, VkShaderModule vert, VkShaderModule frag, VkRenderPass renderPass) {
    //  Attributes for cube
    VkVertexInputBindingDescription bindingDescription =
    {
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    //  Layout for xyz, rgb and st
    VkVertexInputAttributeDescription attributeDescriptions[] =
    {
        { .location=0, .format=VK_FORMAT_R32G32B32_SFLOAT, .offset=offsetof(Vertex,xyz) },
        { .location=1, .format=VK_FORMAT_R32G32B32_SFLOAT, .offset=offsetof(Vertex,nml) },
        { .location=2, .format=VK_FORMAT_R32G32B32_SFLOAT, .offset=offsetof(Vertex,rgb) },
        { .location=3, .format=VK_FORMAT_R32G32_SFLOAT   , .offset=offsetof(Vertex,st)  },
    };
    //  Set input state
    VkPipelineVertexInputStateCreateInfo vertexInputInfo =
    {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount   = sizeof(bindingDescription)/sizeof(VkVertexInputBindingDescription),
    .pVertexBindingDescriptions      = &bindingDescription,
    .vertexAttributeDescriptionCount = sizeof(attributeDescriptions)/sizeof(VkVertexInputAttributeDescription),
    .pVertexAttributeDescriptions    = attributeDescriptions,
    };

    // Create Uniform Buffer
    CreateUniformBuffer(device);

    //  Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
    .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts    = &descriptorSetLayout,
    };
    if (vkCreatePipelineLayout(device,&pipelineLayoutInfo,NULL,&pipelineLayout))
    Fatal("Failed to create pipeline layout\n");

    CreateDescriptorPool(device);

    CreateDescriptorSets(device);

    //  Select TRIANGLE LIST
    VkPipelineInputAssemblyStateCreateInfo inputAssembly =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    //  Enable CULL FACE
    VkPipelineRasterizationStateCreateInfo rasterizer =
    {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .lineWidth               = 1,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
    };

    //  Enable Z-buffer
    VkPipelineDepthStencilStateCreateInfo depthStencil =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    //  Disable multisampling
    VkPipelineMultisampleStateCreateInfo multisampling =
    {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable  = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    //  Disable blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable    = VK_FALSE,
    };

    //  Blend function copy
    VkPipelineColorBlendStateCreateInfo colorBlending =
    {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable     = VK_FALSE,
        .logicOp           = VK_LOGIC_OP_COPY,
        .attachmentCount   = 1,
        .pAttachments      = &colorBlendAttachment,
        .blendConstants[0] = 0,
        .blendConstants[1] = 0,
        .blendConstants[2] = 0,
        .blendConstants[3] = 0,
    };

    //  Enable viewport and scissors test
    VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    //  Allow viewport and scissors to change dynamically
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState =
    {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamicStates)/sizeof(VkDynamicState),
        .pDynamicStates    = dynamicStates,
    };

    //  Vertex shader module
    VkPipelineShaderStageCreateInfo vertShaderStageInfo =
    {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert,
        .pName  = "main",
    };

    //  Fragment shader module
    VkPipelineShaderStageCreateInfo fragShaderStageInfo =
    {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag,
        .pName  = "main",
    };

    //  Create graphics pipeline
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,fragShaderStageInfo};
    VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pDepthStencilState  = &depthStencil,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = renderPass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
    };
    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pipelineInfo,NULL,&graphicsPipeline))
        Fatal("failed to create graphics pipeline\n");

    //  Delete shader modules
    vkDestroyShaderModule(device,frag,NULL);
    vkDestroyShaderModule(device,vert,NULL);

    return graphicsPipeline;
}

// Destroy images
void DestroyImages(VkDevice device) {
   //  Destroy Texture
   vkDestroyImage(device,textureImage,NULL);
   vkFreeMemory(device,textureImageMemory,NULL);
   vkDestroySampler(device,textureSampler,NULL);
   vkDestroyImageView(device,textureImageView,NULL);

   vkDestroyPipelineLayout(device,pipelineLayout,NULL);
}