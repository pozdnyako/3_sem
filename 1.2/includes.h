#ifndef INCLUDES_H
#define INCLUDES_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define checker(a, b) \
if(a) {\
    printf("[ERROR]\t" "%s\n", b);\
    printf("%s\n", strerror(errno));\
    exit(-1);\
}

#define logfilename "backuplog.log"

#include "Tree.h"

int get_children_num(Node *parent, int type, char* name);

typedef struct dirent dirent;
typedef struct tm time_info;
typedef struct stat filestat;

#endif // INCLUDES_H
