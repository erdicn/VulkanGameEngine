#ifndef DEFINES_H
#define DEFINES_H
#define MAX_EXTENSIONS 128
// #define USE_OVERLAY
// #define INCLUDE_OVERLAY
#define USE_IMGUI
// #define NDEBUG

#ifdef __cplusplus
    #define ZERO_INIT {}
#else
    #define ZERO_INIT {0}
#endif

#endif // DEFINES_H