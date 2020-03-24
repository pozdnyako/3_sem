#include "includes.h"

bool is_updated(char* pathF, char* pathT) {
    int fdF = open(pathF, 0, O_RDONLY);
    int fdT = open(pathT, 0, O_RDONLY);

    filestat statF, statT;

    fstat(fdF, &statF);
    fstat(fdT, &statT);

    bool res = !(statF.st_mtime > statT.st_mtime);

    close(fdF);
    close(fdT);

    return res;
}

void cpy(char* pathF, char* pathT) {
    int fdF = open(pathF, O_RDONLY);
    int fdT = open(pathT, O_WRONLY | O_TRUNC);

    off_t filesize = lseek(fdF, 0, SEEK_END);
    lseek(fdF, 0, SEEK_SET);

    char* buf = (char*)calloc(filesize, sizeof(char));
    read(fdF, buf, filesize);
    write(fdT, buf, filesize);

    close(fdF);
    close(fdT);
}

void crt(char* pathF, char* pathT, int _mode, FILE* file) {
    int fdT = creat(pathT, _mode);
    checker((fdT == -1), "failed creating file");
    close(fdT);

    fprintf(file, "[INFO]\t""creat file %s\n", pathT);

    cpy(pathF, pathT);

    printf("%s\n", pathT);

    int p_id = fork();
    if(p_id == 0) {
        int res = execle("./bin/gzip", "./bin/gzip", pathT, (char*)NULL, __envp);

        printf("%d, GZIP error\n", res);
    }
    fprintf(file, "[INFO]\t""copy %s > %s\n", pathF, pathT);
}

void dircrt(Node *treeF, char* pathF, char* pathT, int _mode, FILE* file) {
    mkdir(pathT, _mode);
    fprintf(file, "[INFO]\t""creat dir %s\n", pathT);

    for(int i = 0; i < treeF->n_children; i ++) {
        Node* cur = treeF->children + i;

        char n_pathF[1024] = "";
        strcat(n_pathF, pathF);
        strcat(n_pathF, "/");
        strcat(n_pathF, cur->data);

        char n_pathT[1024] = "";
        strcat(n_pathT, pathT);
        strcat(n_pathT, "/");
        strcat(n_pathT, cur->data);

        filestat statF;
        checker((lstat(n_pathF, &statF) == -1), "failed stat getting");

        mode_t mode = statF.st_mode;

        if(cur->type == DT_DIR) {
            dircrt(cur, n_pathF, n_pathT, mode, file);
        } else {
            crt(n_pathF, n_pathT, mode, file);
        }
    }
}

void _copy(Node *treeF, Node *treeT, char* pathF, char* pathT, FILE *file) {
    if(treeF->type != DT_DIR && !is_updated(pathF, pathT)) {
        cpy(pathF, pathT);
        fprintf(file, "[INFO]\t" "update %s\n", pathT);
    }

    for(int i = 0; i < treeF->n_children; i ++) {
        Node *cur = treeF->children + i;

        char n_pathF[1024] = "";
        strcat(n_pathF, pathF);
        strcat(n_pathF, "/");
        strcat(n_pathF, cur->data);

        char n_pathT[1024] = "";
        strcat(n_pathT, pathT);
        strcat(n_pathT, "/");
        strcat(n_pathT, cur->data);

        int n = get_children_num(treeT, cur->type, cur->data);
        if(n >= 0) {
            _copy(cur, treeT->children + n, n_pathF, n_pathT, file);
        } else {
            filestat statF;
            checker((lstat(n_pathF, &statF) == -1), "failed stat getting");

            mode_t mode = statF.st_mode;

            if(cur->type == DT_DIR) {
                dircrt(cur, n_pathF, n_pathT, mode, file);
            } else {
                crt(n_pathF, n_pathT, mode, file);
            }
        }
    }
}
