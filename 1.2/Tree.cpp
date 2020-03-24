#include "includes.h"
#include "Tree.h"

void Node :: alloc_mem() {
    checker((n_children < 0), "n_children < 0")
    checker((children != NULL), "memory already allocated")
    if(n_children > 0)
        children = (Node*) calloc(n_children, sizeof(Node));
}

#define tab_print(level) \
for(int __ic = 0; __ic < level; __ic ++) {\
    printf("\t");\
}

void Node :: print(int level) {
    tab_print(level)
    switch(type) {
    case DT_DIR: printf("DIR  "); break;
    case DT_REG: printf("REG  "); break;
    }
    printf("<%s>\n", data);

    for(int i = 0; i < n_children; i ++) {
        children[i].print(level + 1);
    }
}
