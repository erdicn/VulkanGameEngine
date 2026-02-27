
INCLUDES = -I./imgui -lglm # -I/home/erdi/Programs/cimgui # -I$(NUKLEAR_DIR) -I$(NUKLEAR_DIR2) # -I$(NUKLEAR_DIR3) 
CFLAGS = -O3 #-Wall # -Wextra -Wno-Wunused-parameter -Wno-Wunused-variable  -Wno-Wunused-const-variable=
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lm  $(INCLUDES)
GLSLC = /home/erdi/Programs/Vulkan/default/x86_64/bin/glslc

CC = g++
SRCS    = $(wildcard *.c) $(wildcard *.cpp) $(wildcard imgui/*.cpp) imgui/backends/imgui_impl_vulkan.cpp imgui/backends/imgui_impl_glfw.cpp
HEADERS = $(wildcard *.h) $(wildcard *.hpp) $(wildcard imgui/*.h)   imgui/backends/imgui_impl_vulkan.h   imgui/backends/imgui_impl_glfw.h
CFLAGS := -Wall

# CC = gcc
# SRCS    = $(wildcard *.c) # $(wildcard *.cpp) $(wildcard imgui/*.cpp) imgui/backends/imgui_impl_vulkan.cpp imgui/backends/imgui_impl_glfw.cpp
# HEADERS = $(wildcard *.h) # $(wildcard *.hpp) $(wildcard imgui/*.h)   imgui/backends/imgui_impl_vulkan.h   imgui/backends/imgui_impl_glfw.h
# CFLAGS := -Wall

# CC = nvcc
# SRCS    = $(wildcard *.c) $(wildcard *.cpp) $(wildcard imgui/*.cpp) imgui/backends/imgui_impl_vulkan.cpp imgui/backends/imgui_impl_glfw.cpp
# HEADERS = $(wildcard *.h) $(wildcard *.hpp) $(wildcard imgui/*.h)   imgui/backends/imgui_impl_vulkan.h   imgui/backends/imgui_impl_glfw.h
# CFLAGS := --compiler-options -Wall



TARGET = VulkanTest

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.cpp=.o)

VERT_SHADERS = $(wildcard *.vert)
FRAG_SHADERS = $(wildcard *.frag)
# This converts .vert to vert.spv and .frag to frag.spv
SPV_FILES = $(VERT_SHADERS:.vert=vert.spv) $(FRAG_SHADERS:.frag=frag.spv)




all: $(TARGET) $(SPV_FILES)

debug: CFLAGS = -g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer
debug: LDFLAGS += -fsanitize=address
debug: clean all

valgrind: debug
	valgrind --leak-check=full \
	         --show-leak-kinds=all \
	         --track-origins=yes \
	         --verbose \
	         ./$(TARGET)


# $(TARGET): $(SRCS) $(BACKEND_OBJS) $(SPV_FILES) # main.c shader.frag shader.vert
# 	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

$(TARGET): $(OBJS) $(SPV_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


############## CIMGUI
cimgui_impl_vulkan.o: $(BACKEND_DIR)/cimgui_impl_vulkan.cpp
	g++ -c $(BACKEND_DIR)/cimgui_impl_vulkan.cpp -o cimgui_impl_vulkan.o $(INCLUDES)

cimgui_impl_glfw.o: $(BACKEND_DIR)/cimgui_impl_glfw.cpp
	g++ -c $(BACKEND_DIR)/cimgui_impl_glfw.cpp -o cimgui_impl_glfw.o $(INCLUDES)
############## CIMGUI



%vert.spv: %.vert
	$(GLSLC) $< -o $@

%frag.spv: %.frag
	$(GLSLC) $< -o $@

.PHONY: test clean

test: all
	./$(TARGET)

clean:
	rm -f $(TARGET) *.spv *.o *.d

cleanall:
	rm -f $(TARGET) *.spv $(OBJS) $(OBJS:.o=.d)