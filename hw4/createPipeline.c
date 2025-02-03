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

// void VertexBindingDescription

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

// //  Copy uniform data to buffer
// void UniformBufferData(const void* src, size_t n) {
//    memcpy(uniformBuffersMapped[currentFrame],src,n);
// }

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

void CreateGraphicsPipeline(VkDevice device, VkShaderModule vert, VkShaderModule frag, VkRenderPass renderPass) {
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
