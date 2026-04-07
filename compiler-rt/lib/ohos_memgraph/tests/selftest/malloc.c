#include <stdlib.h>

int *g_ptr1 = NULL;
int *g_ptr2 = NULL;

void init_memory() {
    g_ptr1 = (int*)malloc(sizeof(int));
    g_ptr2 = (int*)malloc(sizeof(int));
    *g_ptr1 = 100;
    *g_ptr2 = 200;
    g_ptr2 = g_ptr1;
}

int get_ptr1_value(){
    return *g_ptr1;
}

int get_ptr2_value(){
    return *g_ptr2;
}

int* get_ptr1(){
    return g_ptr1;
}

void cleanup(){
    free(g_ptr1);
}
