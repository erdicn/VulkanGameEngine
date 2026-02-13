#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file.h"

char* readFile(const char* filename, size_t* out_size){
    FILE* file = fopen(filename, "rb");

    if (!file) {
        fprintf(stderr, "failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file); 

    char* buffer = (char*)malloc(fileSize);
    if (!buffer) {
        fprintf(stderr, "failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        fprintf(stderr, "failed to read file: %s\n", filename);
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    *out_size = fileSize;
    return buffer;
}
