#ifndef VERTEX_H
#define VERTEX_H

#include <vulkan/vulkan.h>
#include <cglm/cglm.h>

typedef struct Vertex2D {
    vec2 pos;
    vec3 color;
} Vertex2D_t;

typedef struct ArrayVertex2D {
    Vertex2D_t* vert;
    uint32_t len;
} ArrVertex2D_t;



VkVertexInputBindingDescription vert2GetBindingDescription();
// VkVertexInputAttributeDescription* vert2GetAttributeDescriptions(uint32_t* attribute_description_len);
void vert2GetAttributeDescriptions(VkVertexInputAttributeDescription* attribute_descriptions, uint32_t* attribute_description_len);

#endif