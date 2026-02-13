#ifndef DEVICE_H
#define DEVICE_H

#include "defines.h"
#include <vulkan/vulkan.h>
#include "app_structs.h"

extern const char* extensions[MAX_EXTENSIONS];
extern const char* device_extension_names[];
extern csarray_t device_extensions;


bool checkDeviceExtensionSupport(VkPhysicalDevice device);
void pickPhysicalDevice(AppVariables_t* app);
void createLogicalDevice(AppVariables_t* app);

#endif // DEVICE_H