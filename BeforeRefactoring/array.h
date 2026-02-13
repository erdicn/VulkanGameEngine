#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <stdint.h>

typedef struct sarray {
    char** str_arr;
    uint32_t len;
} sarray_t;

typedef struct constsarray {
    const char** str_arr;
    uint32_t len;
} csarray_t;

typedef struct Arruint32{
    uint32_t* vals;
    uint32_t len;
}ArrUint32_t;

#endif //ARRAY_H