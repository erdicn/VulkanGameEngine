#include "app_structs.h"



void collectGarbage(void* garb_addy, AppVariables_t* app){
    if(app->garbabe_len >= MAX_GARBGE_SIZE){
        printf("WARNING: Max garbage len is reached memory will leak%d/%d\n", app->garbabe_len, MAX_GARBGE_SIZE);
        app->garbabe_len++;
        return;
    }
    app->garbage_collector[app->garbabe_len++] = garb_addy;
    if(app->garbabe_len > 0.9*MAX_GARBGE_SIZE ){
        printf("CAREFULL garbage collector at %d/%d\n", app->garbabe_len, MAX_GARBGE_SIZE);
    }    
    return;
}