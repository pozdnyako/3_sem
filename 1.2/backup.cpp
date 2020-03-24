#include "includes.h"

time_info get_time() {
    time_t _time = time(NULL);
    return *localtime(&_time);
}

const int TIME_MAX_LEN = 50;

void print_time(char* buf, time_info _time) {
    strftime(buf, TIME_MAX_LEN, "%a %d-%h-%G %T", &_time);
}

void print_dirent(dirent *dinfo) {
    printf("\t%11d\tInode number\n", (int)dinfo->d_ino);
    printf("\t%11d\tICurrnt position in dir stream\n", (int)dinfo->d_off);
    printf("\t%11d\tLength of this record\n", (int)dinfo->d_reclen);
    printf("\t%11d\tType of file\n", (int)dinfo->d_type);
    printf("\t<%s>\tfilename\n", dinfo->d_name);
}

char *permissions(char* filename) {
    filestat _stat;
    char *mode = (char*)malloc(sizeof(char) * 9 + 1);

    if(stat(filename, &_stat) == 0) {
        mode_t perm = _stat.st_mode;
        mode[0] = (perm & S_IRUSR) ? 'r' : '-';
        mode[1] = (perm & S_IWUSR) ? 'w' : '-';
        mode[2] = (perm & S_IXUSR) ? 'x' : '-';
        mode[3] = (perm & S_IRGRP) ? 'r' : '-';
        mode[4] = (perm & S_IWGRP) ? 'w' : '-';
        mode[5] = (perm & S_IXGRP) ? 'x' : '-';
        mode[6] = (perm & S_IROTH) ? 'r' : '-';
        mode[7] = (perm & S_IWOTH) ? 'w' : '-';
        mode[8] = (perm & S_IXOTH) ? 'x' : '-';
        mode[9] = '\0';

        return mode;
    }
}

bool is_updated(char*, char*);
void cpy(char*, char*);
void crt(char*, char*, char*, FILE*);
void dircrt(Node*, char*, char*, int, FILE*);
void _copy(Node*, Node*, char*, char*, FILE*);
// from copy.cpp

void rm(char*, FILE*);
void dirrm(Node*, char*, FILE*);
void _remove(Node*, Node*, char*, FILE*);
// from remove.cpp

void mkTree(Node *root, DIR *dir, char* dirp) {
    int n_children = 0;
    while(true) {
        dirent* dirinfo = readdir(dir);

        if(dirinfo == NULL) {
            break;
        }

        if(dirinfo->d_type == DT_DIR && (strcmp(dirinfo->d_name, ".")  == 0||
                                         strcmp(dirinfo->d_name, "..") == 0) ) {
            continue;
        }

        n_children ++;
    }

    rewinddir(dir);
    root->n_children = n_children;
    root->alloc_mem();

    { int i = 0;
    while(i < n_children) {
        dirent* dirinfo = readdir(dir);

        if(dirinfo->d_type == DT_DIR && (strcmp(dirinfo->d_name, ".") == 0 ||
                                         strcmp(dirinfo->d_name, "..") == 0) ) {
            continue;
        }

        root->children[i] = Node(dirinfo->d_type, dirinfo->d_name);

        char path[1024] = "";
        strcat(path, dirp);
        strcat(path, "/");
        strcat(path, dirinfo->d_name);

        if(dirinfo->d_type == DT_DIR) {

            DIR *n_dir = opendir(path);
            checker((n_dir == NULL), "dir doesn't exist");

            mkTree(root->children + i, n_dir, path);
            closedir(n_dir);
        }

        printf("%s\t%s\n", permissions(path), path);
        i ++;
    }}
}

int get_children_num(Node *parent, int type, char* name) {
    for(int i = 0; i < parent->n_children; i ++) {
        if(type != parent->children[i].type ||
           strcmp(parent->children[i].data, name) != 0)
           continue;

        return i;
    }

    return -1;
}

//#include "remove.cpp"
//#include "copy.cpp"

void backup(Node *treeF, Node *treeT, char* pathF, char* pathT) {
    char logpath[1024] = "";
    strcat(logpath, pathT);
    strcat(logpath, "/");
    strcat(logpath, logfilename);

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    printf("LOG:%s\n", logpath);

    int log_fd = open(logpath, O_CREAT, mode);
    close(log_fd);


    FILE *file = fopen(logpath, "a");

    time_info start_time = get_time();
    {
    char start_time_str[TIME_MAX_LEN];
    print_time(start_time_str, start_time);
    fprintf(file, "[INFO]\tbackuping started at %s\n", start_time_str);
    }

    _copy(treeF, treeT, pathF, pathT, file);
    _remove(treeF, treeT, pathT, file);

    time_info finish_time = get_time();
    {
    char finish_time_str[TIME_MAX_LEN];
    print_time(finish_time_str, finish_time);
    fprintf(file, "[INFO]\tbackuping finished at %s\n", finish_time_str);
    }
    fprintf(file, "\n\n");

    fclose(file);
}

char** __envp;

int main(int argc, char* argv[], char** envp) {
    __envp = envp;
    checker((argc < 3), "syntaxis: backup dirFrom dirTo")

    char pathF[1024];
    char pathT[1024];


    strcpy(pathF, argv[1]);
    strcpy(pathT, argv[2]);


    DIR *dirF = opendir(pathF);
    checker((dirF == NULL), "dirFrom doesn't exist");

    DIR *dirT = opendir(pathT);
    checker((dirT == NULL), "dirTo doesn't exist");


    Node treeF = Node(DT_DIR, pathF);
    mkTree(&treeF, dirF, pathF);
    treeF.print(0);
    closedir(dirF);

    Node treeT = Node(DT_DIR, pathT);
    mkTree(&treeT, dirT, pathT);
    treeT.print(0);
    closedir(dirT);

    backup(&treeF, &treeT, pathF, pathT);
}

