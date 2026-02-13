#ifndef APP_STRUCTS_H
#define APP_STRUCTS_H

#include "defines.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

// #include "nuklear-glfw-vulkan.h"
// #include "nuklear.h"
#include "overlay.h"
#if defined (__cplusplus ) && defined (USE_IMGUI) 
    #include "imgui/backends/imgui_impl_vulkan.h"
#endif

#include <stdbool.h>
#include <stdio.h>

#include "vertex2D.h"
#include "array.h"

#define MAX_GARBGE_SIZE 2048
#define MAX_FRAMES_IN_FLIGHT 2                         

typedef struct SwapChain{
    VkSwapchainKHR swap_chain;
    VkImage* images;
    uint32_t images_len;
    VkFormat image_format;
    VkExtent2D extend;
    VkImageView* image_views;
    uint32_t image_views_len;
    VkFramebuffer* frame_buffers; 
    uint32_t frame_buffers_len; 
} SwapChain_t;

typedef struct UIVars{
    // struct nk_context *ctx; 
    // struct nk_color background;
    // struct nk_font_atlas *atlas; 

    VkImage* images;
    uint32_t images_len;
    // VkFormat image_format;
    // VkExtent2D extend;
    VkImageView* image_views;
    uint32_t image_views_len;
    VkDeviceMemory* image_memories;
    uint32_t image_memories_len;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkPipelineLayout triangle_pipeline_layout;
    VkPipeline triangle_graphics_pipeline;

    
    overlay_settings_t settings;
}UIVars_t;

typedef struct ImguiVars{
    #if defined (__cplusplus ) && defined (USE_IMGUI) 
        ImGui_ImplVulkanH_Window g_MainWindowData;
        ImDrawData* draw_data;
    #endif
    VkDescriptorPool imgui_descriptor_pool;
    VkClearValue clear_color;
    bool show_demo_window;// = true;
    bool show_another_window;// = false;
} ImguiVars_t;

typedef struct AppVariables{
    GLFWwindow* window;// = initWindow(); // the window which the applicatin will be
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physical_device;// = VK_NULL_HANDLE;
    VkDevice device; 

    const char* extensions[MAX_EXTENSIONS];
    const char** device_extension_names;// = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    csarray_t device_extensions;// = {.len = 1, .str_arr = device_extension_names};
    
    VkQueue graphics_queue;
    uint32_t graphics_queue_family_index;
    VkQueue present_queue; 
    
    SwapChain_t swap_chain;// TODO not the best naming confuses easily with the swapchain var
    
    VkRenderPass render_pass;
    VkPipelineCache pipeline_cache;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkCommandPool   command_pool;
    VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
    // uint32_t        command_buffer_len;

    VkSemaphore  image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore  render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence      in_flight_fences[MAX_FRAMES_IN_FLIGHT]; 
    bool frame_buffer_resized;
    // uint32_t     image_available_semaphores_len;
    // uint32_t     render_finished_semaphores_len;
    // uint32_t     in_flight_fences_len; 

    uint32_t  current_frame;

    ArrVertex2D_t* vertices;
    ArrUint32_t* indices;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    void* vertex_persitent_buffer_mapped_ptr;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    void* staging_buffer_mapped_ptr; 
    
    void*    garbage_collector[MAX_GARBGE_SIZE];
    uint16_t garbabe_len;

    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_Layout;
    VkDescriptorSet* descriptor_sets;
    uint32_t descriptor_sets_len;
    
    ImguiVars_t imgui_vars;
    UIVars_t overlay;

    csarray_t* validation_layers;

}AppVariables_t;

void collectGarbage(void* garb_addy, AppVariables_t* app);

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool hasGraphicsFamily; 
    bool hasPresentFamily; 
}QueueFamilyIndices_t;

typedef struct Shader{
    char* code;
    size_t len;
} Shader_t;



#endif //APP_STRUCTS_H