#include "device.h"
#include "debug_messenger.h"
#include "swapchain.h"

const char* extensions[MAX_EXTENSIONS];
const char* device_extension_names[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
csarray_t device_extensions = {.len = 1, .str_arr = device_extension_names};


bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    VkExtensionProperties availableExtensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, availableExtensions);

    uint32_t extensions_checked = 0;
    for (uint32_t i = 0; i < device_extensions.len; i++) {
        for (uint32_t j = 0; j < extension_count; j++) {
             if(strcmp(availableExtensions[j].extensionName, device_extensions.str_arr[i]) == 0){
                extensions_checked++; 
                break;
             }
        }     
    }

    return (bool) (extensions_checked == device_extensions.len);
}

bool isDeviceSuitable(VkPhysicalDevice device, AppVariables_t* app) {
    QueueFamilyIndices_t indices = findQueueFamilies(device, app);

    bool extensions_supported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails_t swapChainSupport = querySwapChainSupport(device, app);
        swapChainAdequate = (swapChainSupport.formats_len != 0) && (swapChainSupport.present_modes_len != 0);
        cleanupSwapChainSupport(&swapChainSupport);
    }


    return indices.hasGraphicsFamily && indices.hasPresentFamily && extensions_supported && swapChainAdequate;
}  

void pickPhysicalDevice(AppVariables_t* app){
    uint32_t deviceCount = 0;
    testVK(vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL));
    VkPhysicalDevice devices[deviceCount];
    testVK(vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices));

    if (deviceCount == 0) {
        fprintf(stderr, "failed to find GPUs with Vulkan support!");
    }

    for (int i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], app)) {
            app->physical_device = devices[i];
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(app->physical_device, &device_properties);
            printf("Choosen device is: %s\n", device_properties.deviceName);
            break;
        }
    }

    if (app->physical_device == VK_NULL_HANDLE) {
        fprintf(stderr, "failed to find a suitable GPU!");
    }
}

void createLogicalDevice(AppVariables_t* app){
    QueueFamilyIndices_t indices = findQueueFamilies(app->physical_device, app);

    const uint32_t nb_queues = 2;
    VkDeviceQueueCreateInfo queue_create_infos[nb_queues];
    uint32_t unique_indices[nb_queues];
    uint32_t unique_count = 0;

    unique_indices[unique_count++] = indices.graphicsFamily;
    if (indices.graphicsFamily != indices.presentFamily){
        unique_indices[unique_count++] = indices.presentFamily;
    }

    float queue_priority = 1.0f; // influences the command buffer execution priority
    for(uint32_t i = 0; i < unique_count; i++){
        VkDeviceQueueCreateInfo queue_create_info = ZERO_INIT;
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = unique_indices[i]; //indices.graphicsFamily;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos[i] = queue_create_info;
    }

    VkPhysicalDeviceFeatures device_features = ZERO_INIT;
    VkDeviceCreateInfo create_info = ZERO_INIT;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = unique_count;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.pEnabledFeatures  = &device_features;
    create_info.enabledExtensionCount = device_extensions.len;
    create_info.ppEnabledExtensionNames = device_extensions.str_arr;

    if (enableValidationLayers){
        create_info.enabledLayerCount = (uint32_t) app->validation_layers->len;
        create_info.ppEnabledLayerNames = app->validation_layers->str_arr;
    } else {
        create_info.enabledLayerCount = 0;
    }

    testVK(vkCreateDevice(app->physical_device, &create_info, NULL, &app->device));

    vkGetDeviceQueue(app->device, indices.graphicsFamily, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->device, indices.presentFamily , 0, &app->present_queue);
}
