#include "defines.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <stdlib.h>
#include "vertex2D.h"

VkVertexInputBindingDescription vert2GetBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = ZERO_INIT;
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex2D_t);
    // VK_VERTEX_INPUT_RATE_VERTEX:   Move to the next data entry after each vertex
    // VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

void vert2GetAttributeDescriptions(VkVertexInputAttributeDescription* attribute_descriptions, uint32_t* attribute_description_len){
    if(*attribute_description_len != 2){
        fprintf(stderr, "The attribute description is not the correct length (%d) line %d file %s\n", *attribute_description_len, __LINE__, __FILE__);
        abort();
    }
    // *attribute_description_len = 2;
    // attribute_descriptions[2] = ZERO_INIT;

    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    // float: VK_FORMAT_R32_SFLOAT
    // vec2: VK_FORMAT_R32G32_SFLOAT
    // vec3: VK_FORMAT_R32G32B32_SFLOAT
    // vec4: VK_FORMAT_R32G32B32A32_SFLOAT
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // TODO add this to app
    attribute_descriptions[0].offset = offsetof(Vertex2D_t, pos);
    // ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
    // uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
    // double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex2D_t, color);
    // return attribute_descriptions;
}