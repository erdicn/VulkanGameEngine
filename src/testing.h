#ifndef TESTING_H
#define TESTING_H

#include <cglm/vec4.h>
#include <cglm/mat4.h>  

void testCGLM(){
    mat4 matrix;
    vec4 vec;
    vec4 test;
    glm_mat4_mulv(matrix, vec, test);
}

#endif // TESTING_H