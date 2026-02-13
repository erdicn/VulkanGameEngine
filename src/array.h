#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <stdint.h>

typedef struct sarray {
    uint32_t len;
    char** str_arr;
} sarray_t;

typedef struct constsarray {
    uint32_t len;
    const char** str_arr;
} csarray_t;

typedef struct Arruint32{
    uint32_t len;
    uint32_t* vals;
}ArrUint32_t;

#endif //ARRAY_H