#include "defines.h"

#if defined (__cplusplus ) && defined (USE_IMGUI) 
    #include "imgui/imgui.h"
    #include "imgui/backends/imgui_impl_glfw.h"
    #include "imgui/backends/imgui_impl_vulkan.h"
// #else 
    // #define IM_COUNTOF(_ARR)            ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers! Defined in imgui.h
#endif



#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <cglm/vec4.h>
// #include <cglm/mat4.h>  
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// For sleep
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

void mySleep(uint64_t sleep_in_ms){
    //sleep:
    #ifdef _WIN32
    Sleep(pollingDelay);
    #else
    usleep(sleep_in_ms*1000);  /* sleep for sllep_in_ms milliSeconds */
    #endif
}

#include "app_structs.h"
#include "testing.h"
#include "debug_messenger.h" 
// #include "list.h"
#include "array.h"
#include "swapchain.h"
#include "graphics_pipeline.h"
#include "vertex2D.h"
#include "file.h"
#include "device.h"

#ifdef INCLUDE_OVERLAY
#include "overlay.h"
#endif



static const uint32_t WIDTH  = 800;
static const uint32_t HEIGHT = 600;

// TODO might need to get nb extentions in len
#ifndef NDEBUG
static const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
static csarray_t validation_layers = {.len = 1, .str_arr = layers};
#else
static const char* layers[] = ZERO_INIT;
static csarray_t validation_layers = {.len = 0, .str_arr = layers};
#endif


#if defined (__cplusplus ) && defined (USE_IMGUI) 
ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.f);//ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
#endif



// static VkPhysicalDevice physical_device = VK_NULL_HANDLE;
// static VkDevice device; 

// static VkQueue graphics_queue;
// static VkQueue present_queue;

const char* extensions[MAX_EXTENSIONS];

static void framebufferResizeCaalback(GLFWwindow* window, int width, int height){
    // this gets the app that we passed through the glfwSetWindowUserPointer(app->window, app); 
    AppVariables_t* app = (AppVariables_t*) glfwGetWindowUserPointer(window);
    app->frame_buffer_resized = true;
}

void initWindow(AppVariables_t* app){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app); 
    glfwSetFramebufferSizeCallback(app->window, framebufferResizeCaalback);

}

const char** getRequiredExtensions(uint32_t* nb_extentions) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // uint32_t extensionCount = 0;    
    // testVK(vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL));
    // printf("%d extensions supported\n", extensionCount);
    *nb_extentions = glfwExtensionCount + 1; 
    if (*nb_extentions > MAX_EXTENSIONS) {
        fprintf(stderr, "Too many extensions!\n");
        return NULL;
    }
    for(size_t i = 0; i < glfwExtensionCount; i++){
        extensions[i] = (char*) glfwExtensions[i];
    }
    extensions[glfwExtensionCount] = "VK_EXT_debug_utils";

    return extensions;
}

void createInstance(VkInstance* instance){
    VkApplicationInfo app_info = ZERO_INIT;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 4, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 4, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo create_info = ZERO_INIT;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // VkInstanceCreateInfo createInfo;
    // createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pApplicationInfo = &app_info;

    uint32_t nb_max_extensions;
    getRequiredExtensions(&nb_max_extensions);
    create_info.enabledExtensionCount = nb_max_extensions;
    create_info.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        create_info.enabledLayerCount = validation_layers.len;
        create_info.ppEnabledLayerNames = validation_layers.str_arr;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = NULL;
    }

    testVK(vkCreateInstance(&create_info, NULL, instance));
}


void createSurface(AppVariables_t* app){
    testVK(glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface));
}

// void createDescriptorPool(AppVariables_t* app) {
// VkDescriptorPoolSize poolSize = ZERO_INIT;
// poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
// poolSize.descriptorCount =(uint32_t)(app->swap_chain.frame_buffers_len);;

// VkDescriptorPoolCreateInfo poolInfo = ZERO_INIT;
// poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// poolInfo.poolSizeCount = 1;
// poolInfo.pPoolSizes = &poolSize;
// poolInfo.maxSets = (uint32_t)(app->swap_chain.frame_buffers_len);
// testVK(vkCreateDescriptorPool(app->device, &poolInfo, NULL, &app->descriptor_pool));
// }

void createDescriptorPool(AppVariables_t* app) {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, (uint32_t)app->swap_chain.frame_buffers_len },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 } // Add this for ImGui fonts
    };

    VkDescriptorPoolCreateInfo poolInfo = ZERO_INIT;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is critical for ImGui
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; 
    poolInfo.poolSizeCount = 2; // Updated count
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1000 + (uint32_t)app->swap_chain.frame_buffers_len;
    
    testVK(vkCreateDescriptorPool(app->device, &poolInfo, NULL, &app->descriptor_pool));
}

void createDescriptorSetLayout(AppVariables_t* app) {
VkDescriptorSetLayoutBinding overlayLayoutBinding = ZERO_INIT;
overlayLayoutBinding.binding = 0;
overlayLayoutBinding.descriptorCount = 1;
overlayLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
overlayLayoutBinding.pImmutableSamplers = NULL;
overlayLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = ZERO_INIT;
descriptorSetLayoutCreateInfo.sType =
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
descriptorSetLayoutCreateInfo.bindingCount = 1;
descriptorSetLayoutCreateInfo.pBindings = &overlayLayoutBinding;

testVK(vkCreateDescriptorSetLayout(app->device, &descriptorSetLayoutCreateInfo,
                                NULL,
                                &app->descriptor_set_Layout));
}

void createDescriptorSets(AppVariables_t* app) {
uint32_t numFbs = app->swap_chain.frame_buffers_len;
app->descriptor_sets = (VkDescriptorSet*)realloc(app->descriptor_sets, numFbs*sizeof(VkDescriptorSet));

VkDescriptorSetLayout layouts[numFbs];
for (uint32_t i = 0; i < numFbs; i++) {
    layouts[i] = app->descriptor_set_Layout;
}

VkDescriptorSetAllocateInfo allocInfo = ZERO_INIT;
allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
allocInfo.descriptorPool = app->descriptor_pool;
allocInfo.descriptorSetCount = numFbs;
allocInfo.pSetLayouts = layouts;
testVK(vkAllocateDescriptorSets(app->device, &allocInfo, app->descriptor_sets));

for (size_t i = 0; i < numFbs; i++) {
    VkDescriptorImageInfo descriptorImageInfo = ZERO_INIT;
    descriptorImageInfo.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = app->overlay.image_views[i];

    VkWriteDescriptorSet descriptorWrite = ZERO_INIT;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = app->descriptor_sets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(app->device, 1, &descriptorWrite, 0, NULL);
}
}

void createRenderPass(AppVariables_t* app){
    VkAttachmentDescription colorAttachment = ZERO_INIT;
    colorAttachment.format = app->swap_chain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // 1 sampling 
    // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
    // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
    // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
    // VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = ZERO_INIT;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // pInputAttachments: Attachments that are read from a shader
    // pResolveAttachments: Attachments used for multisampling color attachments
    // pDepthStencilAttachment: Attachment for depth and stencil data
    // pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    VkSubpassDescription subpass= ZERO_INIT;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = ZERO_INIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = ZERO_INIT;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    testVK(vkCreateRenderPass(app->device, &renderPassInfo, NULL, &app->render_pass));
}

void createCommandPool(AppVariables_t* app){
    QueueFamilyIndices_t queue_family_indices = findQueueFamilies(app->physical_device, app);

    VkCommandPoolCreateInfo poolInfo = ZERO_INIT;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // we are recording a command buffer every time
    poolInfo.queueFamilyIndex = queue_family_indices.graphicsFamily;

    app->graphics_queue_family_index = queue_family_indices.graphicsFamily;

    testVK(vkCreateCommandPool(app->device, &poolInfo, NULL, &app->command_pool));
}

void createCommandBuffers(AppVariables_t* app){
    // app->command_buffer_len = MAX_FRAMES_IN_FLIGHT;
    // app->command_buffers = malloc(app->command_buffer_len*sizeof(VkCommandBuffer));
    // collectGarbage((void *)&app->command_buffers, app);

    VkCommandBufferAllocateInfo allocInfo = ZERO_INIT;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = app->command_pool;
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;//app->command_buffer_len;

    testVK(vkAllocateCommandBuffers(app->device, &allocInfo, app->command_buffers));
}

void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, AppVariables_t* app){
    VkCommandBufferBeginInfo beginInfo = ZERO_INIT;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    // beginInfo.flags = 0; // Optional
    // beginInfo.pInheritanceInfo = NULL; // Optional

    testVK(vkBeginCommandBuffer(command_buffer, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo = ZERO_INIT;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app->render_pass;
    renderPassInfo.framebuffer = app->swap_chain.frame_buffers[image_index];

    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = app->swap_chain.extend;
    //                         render area |  clear values for VK_ATTACHMENT_LOAD_OP_CLEAR
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphics_pipeline);

    VkViewport viewport = ZERO_INIT;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = (float)(app->swap_chain.extend.width);
    viewport.height = (float)(app->swap_chain.extend.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor= ZERO_INIT;
    scissor.offset.x = 0;
    scissor.offset.y = 0;    
    scissor.extent = app->swap_chain.extend;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {app->vertex_buffer};
    VkDeviceSize offsets[] = ZERO_INIT;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, app->index_buffer, 0, VK_INDEX_TYPE_UINT32);

    // The actual vkCmdDraw function is a bit anticlimactic, but it's so simple because of all the information we specified in advance. It has the following parameters, aside from the command buffer:
    // vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
    // instanceCount: Used for instanced rendering, use 1 if you're not doing that.
    // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
    // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
    // vkCmdDraw(command_buffer, 3, 1, 0, 0);
    // vkCmdDraw(command_buffer, (uint32_t)(app->vertices->len), 1, 0, 0);
    vkCmdDrawIndexed(command_buffer, (uint32_t)(app->indices->len), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    testVK(vkEndCommandBuffer(command_buffer));

}

void createSyncObjects(AppVariables_t* app){
    // app->image_available_semaphores_len = MAX_FRAMES_IN_FLIGHT;
    // app->image_available_semaphores     = malloc(app->image_available_semaphores_len*sizeof(VkSemaphore));
    // app->render_finished_semaphores_len = MAX_FRAMES_IN_FLIGHT;
    // app->render_finished_semaphores     = malloc(app->render_finished_semaphores_len*sizeof(VkSemaphore));
    // collectGarbage((void *)&app->image_available_semaphores, app);
    // collectGarbage((void *)&app->render_finished_semaphores, app);

    VkSemaphoreCreateInfo semaphore_info = ZERO_INIT;
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = ZERO_INIT;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        testVK(vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->image_available_semaphores[i])); 
        testVK(vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->render_finished_semaphores[i])); 
        testVK(vkCreateFence(app->device, &fenceInfo, NULL, &app->in_flight_fences[i])); 
    }
}

uint32_t findMemoryType(AppVariables_t* app, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    testVK(VK_ERROR_UNKNOWN); // TODO add my errors
    return 0; // will abort before comming here 
}

void createBuffer(AppVariables_t* app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = ZERO_INIT;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    testVK(vkCreateBuffer(app->device, &bufferInfo, NULL, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(app->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = ZERO_INIT;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(app, memRequirements.memoryTypeBits, properties);

    testVK(vkAllocateMemory(app->device, &allocInfo, NULL, bufferMemory)); // TODO need to change this to a custom allocator 
    /*TODO It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create a custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions.
    You can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the GPUOpen initiative. However, for this tutorial it's okay to use a separate allocation for every resource, because we won't come close to hitting any of these limits for now.*/

    vkBindBufferMemory(app->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(AppVariables_t* app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size){
    VkCommandBufferAllocateInfo allocInfo = ZERO_INIT;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = app->command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(app->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = ZERO_INIT;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = ZERO_INIT;
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = ZERO_INIT;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(app->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(app->graphics_queue);
    vkFreeCommandBuffers(app->device, app->command_pool, 1, &commandBuffer);

}

void createVertexBuffer(AppVariables_t* app){
    VkDeviceSize buffer_size = sizeof(app->vertices->vert[0]) * app->vertices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    
    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->vertices->vert, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);
    
    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
    // VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->vertex_buffer, &app->vertex_buffer_memory);
    
    copyBuffer(app, staging_buffer, app->vertex_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void createPersistentVertexBuffer(AppVariables_t* app) {
    VkDeviceSize buffer_size = sizeof(Vertex2D_t) * app->vertices->len;

    // We use HOST_VISIBLE and HOST_COHERENT for direct CPU access
    createBuffer(app, buffer_size, 
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &app->vertex_buffer, 
                 &app->vertex_buffer_memory);

    // Map it ONCE and keep the pointer for the lifetime of the app
    vkMapMemory(app->device, app->vertex_buffer_memory, 0, buffer_size, 0, &app->vertex_persitent_buffer_mapped_ptr);
    
    // Initial data upload
    memcpy(app->vertex_persitent_buffer_mapped_ptr, app->vertices->vert, (size_t)buffer_size);
}

// same as the createVertexBufferBut only we dont recreate the vertex buffer 
void updateVertexBuffer(AppVariables_t* app){
    VkDeviceSize buffer_size = sizeof(app->vertices->vert[0]) * app->vertices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->vertices->vert, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);
    
    copyBuffer(app, staging_buffer, app->vertex_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void createIndexBuffer(AppVariables_t* app) {
    VkDeviceSize buffer_size = sizeof(app->indices->vals[0]) * app->indices->len;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void* data;
    vkMapMemory(app->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, app->indices->vals, (size_t) buffer_size);
    vkUnmapMemory(app->device, staging_buffer_memory);

    createBuffer(app, buffer_size,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->index_buffer, &app->index_buffer_memory);

    copyBuffer(app, staging_buffer, app->index_buffer, buffer_size);

    vkDestroyBuffer(app->device, staging_buffer, NULL);
    vkFreeMemory(app->device, staging_buffer_memory, NULL);
}

void initVulkan(AppVariables_t* app){
    app->current_frame = 0;
    app->frame_buffer_resized = false; 
    createInstance(&app->instance);
    setupDebugMessenger(app->instance, &app->debug_messenger);   
    createSurface(app);
    pickPhysicalDevice(app);
    createLogicalDevice(app);
    createSwapChain(app);
    // createSwapChainImageViews(app);
    #ifdef USE_OVERLAY
        createOverlayImages(app);
        createOverlayImageViews(app);
    #endif
    createImageViews(app);
    createRenderPass(app);

    createFramebuffers(app); 

    createDescriptorPool(app);
     #ifdef USE_OVERLAY
        createDescriptorSetLayout(app);
        createDescriptorPool(app);
        createDescriptorSets(app);
    #endif
    createGraphicsPipeline(app);

    #ifdef USE_OVERLAY  
        app->overlay.graphics_pipeline = app->graphics_pipeline;
        // createOverlayGraphicsPipeline(app);
    #endif
    

    createCommandPool(app);
    createVertexBuffer(app);
    createIndexBuffer(app);
    // createPersistentVertexBuffer(app);

    createCommandBuffers(app);
    createSyncObjects(app);

        
    #ifdef INCLUDE_OVERLAY
        init_overlay(app);
    #endif
}   

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < validation_layers.len; i++) {
        const char* layerName = validation_layers.str_arr[i];
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void drawFrame(AppVariables_t* app){
    // The vkWaitForFences function takes an array of fences and waits on the host for either any or all of the fences to be signaled before returning. The VK_TRUE we pass here indicates that we want to wait for all fences, but in the case of a single one it doesn't matter. This function also has a timeout parameter that we set to the maximum value of a 64 bit unsigned integer, UINT64_MAX, which effectively disables the timeout.
    vkWaitForFences(app->device, 1, &app->in_flight_fences[app->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(app->device, app->swap_chain.swap_chain, UINT64_MAX, app->image_available_semaphores[app->current_frame], VK_NULL_HANDLE, &image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain(app);
        return;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        testVK(result);
    }

    #ifdef INCLUDE_OVERLAY
        float old_bg_color[4] = {app->overlay.settings.bg_color[0], app->overlay.settings.bg_color[1],
                                 app->overlay.settings.bg_color[2], app->overlay.settings.bg_color[3]};
        VkSemaphore overlayFinished = submit_overlay(app, &app->overlay.settings, app->graphics_queue, image_index, NULL);
    #endif

    vkResetFences(app->device, 1, &app->in_flight_fences[app->current_frame]);
    
    //  vkResetCommandPool(app->device, app->command_pool, 0);

    vkResetCommandBuffer(app->command_buffers[app->current_frame], 0);
    // fprintf(stderr, "POTATO\n");
    // fprintf(stderr, "%d Can we come here ?\n", __LINE__);
    VkSubmitInfo submitInfo = ZERO_INIT;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


    #if defined (__cplusplus ) && defined (USE_IMGUI) 
        VkCommandBufferBeginInfo beginInfo = ZERO_INIT;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        testVK(vkBeginCommandBuffer(app->command_buffers[app->current_frame], &beginInfo));
        
        
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = app->render_pass;
        info.framebuffer = app->swap_chain.frame_buffers[image_index];
        info.renderArea.extent.width = app->swap_chain.extend.width;
        info.renderArea.extent.height = app->swap_chain.extend.height;
        info.clearValueCount = 1;
        info.pClearValues = &app->imgui_vars.clear_color;
        vkCmdBeginRenderPass(app->command_buffers[app->current_frame], &info, VK_SUBPASS_CONTENTS_INLINE);

        {
            vkCmdBindPipeline(app->command_buffers[app->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphics_pipeline);
            VkViewport viewport = ZERO_INIT;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width  = (float)(app->swap_chain.extend.width);
            viewport.height = (float)(app->swap_chain.extend.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(app->command_buffers[app->current_frame], 0, 1, &viewport);

            VkRect2D scissor= ZERO_INIT;
            scissor.offset.x = 0;
            scissor.offset.y = 0;    
            scissor.extent = app->swap_chain.extend;
            vkCmdSetScissor(app->command_buffers[app->current_frame], 0, 1, &scissor);

            VkBuffer vertexBuffers[] = {app->vertex_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(app->command_buffers[app->current_frame], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(app->command_buffers[app->current_frame], app->index_buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(app->command_buffers[app->current_frame], (uint32_t)(app->indices->len), 1, 0, 0, 0);
        }


        ImGui_ImplVulkan_RenderDrawData(app->imgui_vars.draw_data, app->command_buffers[app->current_frame]);
        vkCmdEndRenderPass(app->command_buffers[app->current_frame]);
        VkSemaphore signalSemaphores[] = {app->render_finished_semaphores[image_index]};
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            // VkSubmitInfo info = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &app->image_available_semaphores[app->current_frame];
            submitInfo.pWaitDstStageMask = &wait_stage;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &app->command_buffers[app->current_frame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;
            // fprintf(stderr, "%d Can we come here ?\n", __LINE__);
            testVK(vkEndCommandBuffer(app->command_buffers[app->current_frame]));
            // testVK(vkQueueSubmit(app->graphics_queue, 1, &info, app->in_flight_fences[app->current_frame]));
        }
        // fprintf(stderr, "%d Can we come here ?\n", __LINE__);

    #else 
        recordCommandBuffer(app->command_buffers[app->current_frame], image_index, app);
        VkSemaphore waitSemaphores[] = {app->image_available_semaphores[app->current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {app->render_finished_semaphores[image_index]};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &app->command_buffers[app->current_frame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    #endif // USE_IMGUI

    vkQueueSubmit(app->graphics_queue, 1, &submitInfo, app->in_flight_fences[app->current_frame]);

    VkPresentInfoKHR presentInfo = ZERO_INIT;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {app->swap_chain.swap_chain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &image_index;
    presentInfo.pResults = NULL;


    result = vkQueuePresentKHR(app->present_queue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || 
       app->frame_buffer_resized){
        app->frame_buffer_resized = false;
        recreateSwapChain(app);
    } else if(result != VK_SUCCESS){
        testVK(result);
    }

    app->current_frame = (app->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    #ifdef INCLUDE_OVERLAY
        vkQueueWaitIdle(app->present_queue);
    #endif
}

static double lastTime = 0;
static int frame_count = 0;
static char title[16];

void updateFPS(AppVariables_t* app) {
    // Get current time in seconds
    
    double currentTime = (double)glfwGetTime();
    frame_count++;

    // If one second has passed
    if (currentTime - lastTime >= 3) {
        printf("FPS: %f\n", frame_count/3.);
        fflush(stdout); 
        // sprintf(title, "%d", frame_count);
        glfwSetWindowTitle(app->window, title);
        frame_count = 0;
        lastTime = currentTime;
    }
}


static int pressed_key = -1;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    pressed_key = key;
}

uint64_t potato = 0;
bool wireframe = false;
void stuffTodoEveryFrame(AppVariables_t* app){
    int state = glfwGetKey(app->window, GLFW_KEY_E);
    if (state == GLFW_PRESS)
    {
        // printf("E key pressed\n");
        // glfwWaitEventsTimeout(1); // TODO better sleep function so one input is enough
        vkDeviceWaitIdle(app->device);
        vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
        vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
        if(wireframe) createGraphicsPipelineCustom(app, &app->graphics_pipeline, &app->pipeline_layout, VK_POLYGON_MODE_LINE, "shadervert.spv", "shaderfrag.spv");
        else createGraphicsPipelineCustom(app, &app->graphics_pipeline, &app->pipeline_layout, VK_POLYGON_MODE_FILL, "shadervert.spv", "shaderfrag.spv");
        wireframe = !wireframe;
        mySleep(200); // Since when we change to wireframe we need it ocasionally we can afford to sleep better TODO better alternative with glfw
    }
    glfwSetKeyCallback(app->window, keyCallback);
    
    switch (pressed_key){
        case GLFW_KEY_RIGHT:
            for(size_t i = 0; i < app->vertices->len; i++){
                        app->vertices->vert[i].pos[0] += 0.001;
            }
            break;
        case GLFW_KEY_LEFT:
            for(size_t i = 0; i < app->vertices->len; i++){
                        app->vertices->vert[i].pos[0] -= 0.001;
            }
            break;
        case GLFW_KEY_UP:
            for(size_t i = 0; i < app->vertices->len; i++){
                        app->vertices->vert[i].pos[1] -= 0.001;
            }
            break;
        case GLFW_KEY_DOWN:
            for(size_t i = 0; i < app->vertices->len; i++){
                        app->vertices->vert[i].pos[1] += 0.001;
            }
            break;
        
        default:
            break;
    }
    // pressed_key = -1;

    updateVertexBuffer(app);

    drawFrame(app);
    #ifndef __cplusplus
        updateFPS(app);
    #endif
}

void mainLoop(AppVariables_t* app){
    while(!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();

        stuffTodoEveryFrame(app);
    }

    vkDeviceWaitIdle(app->device);
} 


void cleanup(AppVariables_t* app){
    #ifdef INCLUDE_OVERLAY
        shutdown_overlay();
    #endif
    #if defined (__cplusplus ) && defined (USE_IMGUI)
        ImGui_ImplVulkan_Shutdown();
    #endif

    cleanupSwapChain(app);

    vkDestroyBuffer(app->device, app->index_buffer, NULL);
    vkFreeMemory(app->device, app->index_buffer_memory, NULL);
    
    // if (app->vertex_persitent_buffer_mapped_ptr != NULL) {
    //     vkUnmapMemory(app->device, app->vertex_buffer_memory);
    //     app->vertex_persitent_buffer_mapped_ptr = NULL; 
    // }    
    // vkDestroyBuffer(app->device, app->vertex_persitent_buffer_mapped_ptr, NULL);
    
    vkDestroyBuffer(app->device, app->vertex_buffer, NULL);
    vkFreeMemory(app->device, app->vertex_buffer_memory, NULL);
    

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(app->device, app->image_available_semaphores[i], NULL);
        vkDestroySemaphore(app->device, app->render_finished_semaphores[i], NULL);
        vkDestroyFence(app->device, app->in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(app->device, app->command_pool, NULL);
    vkDestroyDescriptorPool(app->device, app->descriptor_pool, NULL);
    
    vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
    vkDestroyRenderPass(app->device, app->render_pass, NULL);
    
    // for(size_t i = 0; i < app->swap_chain.frame_buffers_len; i++){
    //     vkDestroyFramebuffer(app->device, app->swap_chain.frame_buffers[i], NULL);
    // }
    // for (size_t i = 0; i < app->swap_chain.image_views_len; i++) {
    //     vkDestroyImageView(app->device, app->swap_chain.image_views[i], NULL);
    // }
    // vkDestroySwapchainKHR(app->device, app->swap_chain.swap_chain, NULL);
    
    vkDestroyDevice(app->device, NULL);
    if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(app->instance, app->debug_messenger, NULL);
    }

    vkDestroySurfaceKHR(app->instance, app->surface, NULL);
    vkDestroyInstance(app->instance, NULL);

    freeSwapchainVars(&app->swap_chain);

    glfwDestroyWindow(app->window);
    glfwTerminate();
}

uint32_t vertices_len = 4;
Vertex2D_t vertices[] = {
    // {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    // {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    // {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    // {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    // {{0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    // {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    // {{0.0f, -0.4f}, {1.0f, 1.0f/2, 1.0f}},
    // {{0.4f,  0.4f}, {0.0f, 1.0f/2, 0.0f}},
    // {{-0.4f, 0.4f}, {0.0f, 0.0f, 0.5f}}
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

uint32_t indice_vals[] = {0, 1, 2, 2, 3, 0};
ArrUint32_t indices = { .len = 6, .vals = indice_vals};

#if defined (__cplusplus ) && defined (USE_IMGUI) 
void InitImGuiAndMainLoop(AppVariables_t* app){
    IMGUI_CHECKVERSION();

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(app->window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = app->instance;
    init_info.PhysicalDevice = app->physical_device;
    init_info.Device = app->device;
    init_info.QueueFamily = app->graphics_queue_family_index;
    init_info.Queue = app->graphics_queue;
    init_info.PipelineCache = app->pipeline_cache;
    init_info.DescriptorPool = app->descriptor_pool;
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = app->swap_chain.images_len;
    init_info.Allocator = NULL;
    init_info.PipelineInfoMain.RenderPass = app->render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = testVulkanOnlyErr;
    ImGui_ImplVulkan_Init(&init_info);

    // // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will call AddFontDefault() to select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
    app->imgui_vars.show_demo_window = true;
    app->imgui_vars.show_another_window = false;

    // TODO make it so the simulation runs with window closed too
    // Main loop
    int i = 0;
    while (!glfwWindowShouldClose(app->window))
    {   
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(app->window, &fb_width, &fb_height);
        // if (fb_width > 0 && fb_height > 0 && (g_SwapChainRebuild || g_MainWindowData.Width != fb_width || g_MainWindowData.Height != fb_height))
        // {
        //     ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
        //     ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount, 0);
        //     g_MainWindowData.FrameIndex = 0;
        //     g_SwapChainRebuild = false;
        // }
        if (glfwGetWindowAttrib(app->window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (app->imgui_vars.show_demo_window)
            ImGui::ShowDemoWindow(&app->imgui_vars.show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &app->imgui_vars.show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &app->imgui_vars.show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (app->imgui_vars.show_another_window)
        {
            ImGui::Begin("Another Window", &app->imgui_vars.show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                app->imgui_vars.show_another_window = false;
            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            app->imgui_vars.clear_color.color.float32[0] = clear_color.x * clear_color.w;
            app->imgui_vars.clear_color.color.float32[1] = clear_color.y * clear_color.w;
            app->imgui_vars.clear_color.color.float32[2] = clear_color.z * clear_color.w;
            app->imgui_vars.clear_color.color.float32[3] = clear_color.w;
            // FrameRender(wd, draw_data);
            app->imgui_vars.draw_data = draw_data;
            // FramePresent(wd);
            
            stuffTodoEveryFrame(app);
        }

    }
    vkDeviceWaitIdle(app->device);
}
#endif





void initApp(AppVariables_t* app){
    if (app == NULL) return;
    app->vertices = (ArrVertex2D_t*)malloc(sizeof(*app->vertices));
    app->vertices->vert = vertices;
    app->vertices->len  = vertices_len;
    app->indices = (ArrUint32_t*)malloc(sizeof(*app->indices));
    app->indices->vals = indice_vals;
    app->indices->len = indices.len;
    app->validation_layers = (csarray_t*)malloc(sizeof(csarray_t));
    app->validation_layers->str_arr = validation_layers.str_arr;
    app->validation_layers->len     = validation_layers.len;
    // printf("%d %d %d %d %d\n", app->indices->vals[0], app->indices->vals[1], app->indices->vals[2], app->indices->vals[3], app->indices->len);
}

void freeApp(AppVariables_t* app){
    free(app->vertices);
    free(app->indices);
    free(app->validation_layers);
    // free(app);
}

int mainVulkan(){
    // testCGLM();
    
    AppVariables_t app_vars = ZERO_INIT;
    initApp(&app_vars);
    initWindow(&app_vars);
    app_vars.physical_device = VK_NULL_HANDLE;
    printf("Initialising vulkan\n");
    initVulkan(&app_vars);
    printf("Entering main loop\n");
    #if defined (__cplusplus ) && defined (USE_IMGUI)
        InitImGuiAndMainLoop(&app_vars);
    #else
        mainLoop(&app_vars); 
    #endif

    printf("Cleanup started\n");
    cleanup(&app_vars);
    freeApp(&app_vars);
    return 0;
}

int main(){
    // mainGUI();
    mainVulkan();
    return 0;
}