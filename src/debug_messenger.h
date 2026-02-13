#ifndef DEBUG_MESSENGER_H
#define DEBUG_MESSENGER_H  

// #define NDEBUG

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


static inline void testVulkan(int vk_err, int line, const char* file){
    if(vk_err != VK_SUCCESS){
        fprintf(stderr, "Error (%d) found in line %d in file %s\n", vk_err, line, file);
        abort(); // TODO not sure if abort is the best
    }
    return;
} 

static inline void testVulkanOnlyErr(VkResult vk_err){
    if(vk_err != VK_SUCCESS){
        fprintf(stderr, "Error (%d) found only error entered\n", vk_err);
        // abort();
    }
    return;
} 


#define testVK(vk_err) testVulkan(vk_err, __LINE__, __FILE__)

extern const bool enableValidationLayers;

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
// static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
#endif// DEBUG_MESSENGER_H