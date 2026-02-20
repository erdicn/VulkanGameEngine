#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.h>
#include "app_structs.h"

// void createOverlayGraphicsPipeline(AppVariables_t* app);
// VkShaderModule createShaderModule(AppVariables_t* app, Shader_t* code);
VkShaderModule createShaderModule(VkDevice device, Shader_t* code);
void createGraphicsPipeline(AppVariables_t* app);
// void createGraphicsPipelineCustom(AppVariables_t* app, VkPipeline* new_pipeline, VkPolygonMode polygon_mode, const char* vert_shader, const char* frag_shader);
void createGraphicsPipelineCustom(AppVariables_t* app, VkPipeline* new_pipeline, VkPipelineLayout* new_pipeline_layout, VkPolygonMode polygon_mode, const char* vert_shader, const char* frag_shader);


#endif