#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<cstring>

#define MAX_LEN 300
#define MAX_LEN__ 50

struct PInfo{
    char* str;

    char* path;
    char** args;

    int n_args;

    double time;
};

long get_file_size(FILE *file) {
    fpos_t cur_pos;
    fgetpos(file, &cur_pos);

    fseek(file, 0, SEEK_END);

    fpos_t res;
    fgetpos(file, &res);

    fseek(file, 0, cur_pos.__pos);

    return res.__pos;
}

int read_pinfo(FILE* file, PInfo *pinfo) {
    pinfo->str = (char*) calloc(MAX_LEN, sizeof(char));

    int res = fscanf(file, "<%lf>", &pinfo->time);

    fgets(pinfo->str, MAX_LEN, file);

    if(res != 1){
        return res;
    }
    printf("<%2f>\tstr:'%s'\n", pinfo->time, pinfo->str);


    int len = strlen(pinfo->str);

    pinfo->str[len-1] = '\0';

    len --;
    int n_Tokens = 0;

    if(len <= 2 || (len > 2 && (pinfo->str[0] != '<' || pinfo->str[len - 1] != '>')) ) {
        printf("[ERROR]\t" "syntax\n");
        return -1;
    }

    for(int i = 2; i < len - 2; i ++) {
        if(pinfo->str[i] == ' ' && pinfo->str[i - 1] != ' ')
            n_Tokens ++;
    }
    if(pinfo->str[len - 2] != ' ') {
        n_Tokens ++;
    }
    printf("n tokens: %d\n", n_Tokens);

    pinfo->n_args = n_Tokens + 1;
    pinfo->args = (char**) calloc(pinfo->n_args, sizeof(char*) );

    for(int i = 1; i < len-2; i ++) {
        if(pinfo->str[i] == ' ')
            pinfo->str[i] = '\0';
    }
    pinfo->str[len-1] = '\0';

    int pos = 1;

    for(int i = 0; i < pinfo->n_args - 1; i ++) {
        while(pinfo->str[pos] == '\0') {
            pos ++;
        }

        pinfo->args[i] = pinfo->str + pos;

        while(pinfo->str[pos] != '\0') {
            pos ++;
        }
    }

    pinfo->args[pinfo->n_args - 1] = NULL;

    pinfo->path = pinfo->str + 1;

    printf("prog:\t%s\n", pinfo->path);
    for(int i = 0; i < pinfo->n_args; i ++) {
        printf("arg %3d:\t%s\n", i, pinfo->args[i]);
    }
    printf("\n");

    return res;
}

int main(int argc, char *argv[], char *envp[]) {
    FILE *file = fopen("data.txt", "rt");

    const int P_MAX_N = 50;

    PInfo pinfo[P_MAX_N];
    long fsize = get_file_size(file);

    int p_n = 0;

    for(int i = 0; i < P_MAX_N; i ++) {
        int res = read_pinfo(file, pinfo + p_n);

        if(res != 1) {
            int cur = ftell(file);

            if(cur == fsize) {
                break;
            } else {
                //fseek(file, cur+1, SEEK_SET);
            }
        } else {
            p_n ++;
        }
    }
    fclose(file);

    //printf("[INFO]\t" " %d process read\n", p_n);

    int p_id = -1;
    pid_t *pid = (pid_t*) calloc(p_n, sizeof(pid_t));

    int l_pid = 0;

    for(int i = 0; i < p_n ; i ++) {
        l_pid = fork();

        if(l_pid == 0) {
            p_id = i;
            break;
        } else {
            pid[i] = l_pid;
        }
    }

    if(l_pid == 0) {
        printf("time=%4.2lf\tpath=<%s>\t", pinfo[p_id].time, pinfo[p_id].path);

        for(int i = 1;i < pinfo[p_id].n_args; i ++) {
            printf("'%s' ", pinfo[p_id].args[i]);
        }
        printf("\n");

        sleep((int)pinfo[p_id].time);
        execvpe(pinfo[p_id].path, pinfo[p_id].args, envp);
        printf("[ERROR] -> ");
        printf("time=%4.2lf\tpath=<%s>\t", pinfo[p_id].time, pinfo[p_id].path);

        exit(-1);
    } else {
        int wstatus;
        for(int i = 0; i < p_n; i ++) {
            waitpid(pid[i], &wstatus, 0);
        }

        printf("[INFO]\t" "end\n");

    }

    exit(0);
    return 0;
}



