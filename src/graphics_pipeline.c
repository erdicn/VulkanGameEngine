#include <vulkan/vulkan.h>
#include "debug_messenger.h"
#include "app_structs.h"
#include "graphics_pipeline.h"
#include "file.h"

static VkShaderModule createShaderModule(AppVariables_t* app, Shader_t* code) {
    VkShaderModuleCreateInfo createInfo={};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code->len;
    createInfo.pCode = (const uint32_t*)(code->code);

    VkShaderModule shaderModule;
    testVK(vkCreateShaderModule(app->device, &createInfo, NULL, &shaderModule));

    return shaderModule;
}

void createOverlayGraphicsPipeline(AppVariables_t* app) {
    Shader_t vertShaderCode , fragShaderCode ;
    vertShaderCode.code = readFile("nuklearvert.spv", &vertShaderCode.len);
    fragShaderCode.code = readFile("nuklearfrag.spv", &fragShaderCode.len);

    VkShaderModule vertShaderModule = createShaderModule(app, &vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(app, &fragShaderCode);
    free(vertShaderCode.code);
    free(fragShaderCode.code);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)app->swap_chain.extend.width;
    viewport.height = (float)app->swap_chain.extend.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = app->swap_chain.extend;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor =VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor =VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 1.0f;
    colorBlending.blendConstants[1] = 1.0f;
    colorBlending.blendConstants[2] = 1.0f;
    colorBlending.blendConstants[3] = 1.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &app->descriptor_set_Layout;

    testVK(vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL,
                               &app->overlay.pipeline_layout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = app->overlay.pipeline_layout;
    pipelineInfo.renderPass = app->render_pass;
    pipelineInfo.subpass = 1; 
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    testVK(vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  NULL,
                                  &app->overlay.graphics_pipeline)); 

    vkDestroyShaderModule(app->device, fragShaderModule, NULL);
    vkDestroyShaderModule(app->device, vertShaderModule, NULL);
  }

