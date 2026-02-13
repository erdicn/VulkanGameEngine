#ifndef LIST_H
#define LIST_H  

#include <stdlib.h>

typedef struct slist {
    char* str;
    struct slist* next;
} slist_t;

slist_t* allocateSlist(slist_t* l){
    if (l == NULL)
        l = malloc(sizeof(slist_t));
    l->next = NULL;
    l->str  = NULL; 
    return l;
}

slist_t* appendSlist(slist_t* l, slist_t* add){
    if (add == NULL) return l;
    if (l   == NULL) return add;
    slist_t* start = l;
    while(l->next != NULL){
        l = l->next;
    }
    l->next = add;
    return start;
}

slist_t* freeSlist(slist_t* l){
    slist_t* tmp;
    while(l != NULL){
        tmp->next = l->next;
        free(l->str);
        free(l);
        l = tmp;
    }
    return NULL;    
}

#endif // LIST_H
