#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<cstring>

#define MAX_LEN 300

struct PInfo{
    char path[50];

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
    char* input = (char*) calloc(MAX_LEN, sizeof(char));

    int res = fscanf(file, "<%lf>", &pinfo->time);

    input = fgets(input, MAX_LEN, file);

    printf("res:%d\n", res);

    if(res != 1){
        return res;
    }

    int len = strlen(input);

    input[len-1] = '\0';

    printf("<%f>%s\n", pinfo->time, input);
    printf("'%s'\n", input);


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

            printf("cur:%d\n", cur);

            if(cur == fsize) {
                break;
            } else {
                fseek(file, cur+1, SEEK_SET);
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
        //printf("time=%4.2lf\tpath=<%s>\n", pinfo[p_id].time, pinfo[p_id].path);
        sleep((int)pinfo[p_id].time);
        execle(pinfo[p_id].path, pinfo[p_id].path, (char*)NULL, envp);
        //printf("[ERROR]\n");
        exit(-1);
    } else {
        int wstatus;
        for(int i = 0; i < p_n; i ++) {
            waitpid(pid[i], &wstatus, 0);
        }

        //printf("[INFO]\t" "end\n");

    }

    exit(0);
    return 0;
}



