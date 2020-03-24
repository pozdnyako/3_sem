#ifndef TREE_H
#define TREE_H

struct Node {
    char data[256];
    int type;

    int n_children;
    Node* children;

    Node()
    :type(-1),
    children(NULL),
    n_children(-1){}

    Node(int _type, char* _data)
    :type(_type),
    children(NULL),
    n_children(-1){
        strcpy(data, _data);
    }

    ~Node(){
        free(children);
    }

    void alloc_mem();
    void print(int);
};

#endif // TREE_H
