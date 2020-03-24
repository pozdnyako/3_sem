#include "includes.h"

void rm(char* path, FILE *file) {
    checker((unlink(path) == -1), "unlink error");
    fprintf(file, "[INFO]\tfile %s removed\n", path);
}

void dirrm(Node *tree, char* path, FILE *file) {
    for(int i = 0; i < tree->n_children; i ++) {
        Node *cur = tree->children + i;

        char n_path[1024] = "";
        strcat(n_path, path);
        strcat(n_path, "/");
        strcat(n_path, cur->data);

        if(cur->type == DT_DIR) {
            dirrm(cur, n_path, file);
        } else {
            rm(n_path, file);
        }
    }
    rmdir(path);
    fprintf(file, "[INFO]\tdir %s removed\n", path);
}

void _remove(Node *treeF, Node *treeT, char* pathT, FILE *file) {
    for(int i = 0; i < treeT->n_children; i ++) {
        Node *cur = treeT->children + i;
        char n_pathT[1024] = "";
        strcat(n_pathT, pathT);
        strcat(n_pathT, "/");
        strcat(n_pathT, cur->data);

        int n = get_children_num(treeF, cur->type, cur->data);
        if(n >= 0) {
            _remove(treeF->children + n, cur, n_pathT, file);
        } else {
            if(cur->type == DT_DIR) {
                dirrm(cur, n_pathT, file);
            } else {
                if(strcmp(cur->data, logfilename) == 0)
                    continue;

                rm(n_pathT, file);
            }
        }
    }
}
