#include <stdio.h>
#include "memory_unreachable_1.h"
#include "../../memgraph_interface.h"

int main() {
    block_info_t ptr_info;
    init_memory();

    printf("通过g_ptr1访问的值：%d\n",get_ptr1_value());
    printf("通过g_ptr2访问的值：%d\n",get_ptr2_value());
    
    int* get_ptr = get_ptr1();
    get_block_info((unsigned long)get_ptr, &ptr_info);
    printf(ptr_info.name);

    //cleanup();
    return 0;
}
