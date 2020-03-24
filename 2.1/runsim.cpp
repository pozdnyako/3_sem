#include<cstdio>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<cstring>

#include<libexplain/waitpid.h>

#define MAX_LEN 300
#define MAX_LEN__ 50


struct PInfo{
    char** args;
    int n_args;
};

int read_pinfo(char* buffer, PInfo *pinfo) {
    int len = strlen(buffer);

    #define STATE_WORD 1
    #define STATE_SPACE 0
    int state = buffer[0] == ' ' ? STATE_SPACE : STATE_WORD;

    int n_Tokens = 0;

    for(int i = 0; i < len; i ++) {
        if(buffer[i] == ' ') {
            if(state == STATE_WORD) {
                n_Tokens ++;
            }

            state = STATE_SPACE;
        } else {

            state = STATE_WORD;
        }
    }

    if(state == STATE_WORD) {
        n_Tokens ++;
    }

    pinfo->n_args = n_Tokens;
    pinfo->args = (char**) calloc(pinfo->n_args, sizeof(char*));

    state = buffer[0] == ' ' ? STATE_SPACE : STATE_WORD;

    int counter = 0;
    if(state == STATE_WORD) {
        pinfo->args[counter] = buffer;
        counter ++;
    }

    for(int i = 0; i < len; i ++) {
        if(buffer[i] == ' ') {
            state = STATE_SPACE;
            buffer[i] = '\0';
        } else {
            if(state == STATE_SPACE) {
                pinfo->args[counter] = buffer + i;
                counter ++;
            }

            state = STATE_WORD;
        }
    }
}

void DtorPInfo(PInfo* pinfo) {
    free(pinfo->args);
}

struct ShMem {
    char path[50];
    int ftok_param;

    int size;
    void* ptr;

    key_t key;
    int shmid;
};

void *create_shmem(ShMem *pm) {
    pm->key = ftok(pm->path, pm->ftok_param);
    if(pm->key == -1) printf("ERROR\tftok\n");

    pm->shmid = shmget(pm->key, pm->size, 0777 | IPC_CREAT);
    if(pm->shmid == -1) printf("ERROR\tshmid\n");

    pm->ptr = shmat(pm->shmid, NULL, 0);
    if(pm->ptr == (void*)-1) printf("ERROR\tshmat\n");

    printf("shmem attached\n");

    return pm->ptr;
}

void detach_shmem(ShMem *pm) {
    shmdt(pm->ptr);

    printf("shmem dettached\n");
}

void shmem_init(ShMem *shmem, int MAX_N_P) {
    shmem[0].size = MAX_LEN * sizeof(char);
    strcpy(shmem[0].path, "runsim.cpp");
    shmem[0].ftok_param = 0;

    shmem[1].size = 3 * sizeof(int);
    strcpy(shmem[1].path, "runsim.cpp");
    shmem[1].ftok_param = 1;

    shmem[2].size = sizeof(bool) * 2;
    strcpy(shmem[2].path, "runsim.cpp");
    shmem[2].ftok_param = 2;

    shmem[3].size = MAX_N_P * sizeof(pid_t);
    strcpy(shmem[3].path, "runsim.cpp");
    shmem[3].ftok_param = 3;
}

#define MSG_CLR 0
#define MSG_SNT 1
#define MSG_WRG 2

#define MSG_LAUNCH 1
#define MSG_LAUNCHED 2

int main(int argc, char *argv[], char *envp[]) {
    int MAX_N_P = 10;

    if(argc == 1) {
        printf("MAX_N_P = %d by default\n", MAX_N_P);
    } else {
        MAX_N_P = atoi(argv[1]);

        printf("MAX_N_P = %d\n", MAX_N_P);
    }

    ShMem shmem[4];
    shmem_init(shmem, MAX_N_P);

    bool *gen_msg = (bool*) create_shmem(shmem + 2);
    bool *is_exit = gen_msg;
    bool *wrt_log = gen_msg + 1;
    *is_exit = false;
    *wrt_log = false;

    int *msg = (int*) create_shmem(shmem + 1);
    msg[0] = MSG_CLR;
    msg[1] = MSG_CLR;
    msg[2] = MSG_CLR;

    char* buffer = (char*) create_shmem(shmem);

    pid_t p_id = -1;
    pid_t *pids = (pid_t*) create_shmem(shmem + 3);

    int n_p = 0;

    int m_pid = fork();
    PInfo *pinfo = new PInfo;

    if(m_pid > 0) {
        // status checker and launcher
        while(!*is_exit) {

            for(int i = 0; i < MAX_N_P; i ++) {
                int status = 0;

                if(pids[i] > 0) {
                    if(*wrt_log) printf("%d: id= %d ", i, pids[i]);

                    int wstat = waitpid(pids[i], &status, WCONTINUED | WNOHANG);

                    bool is_clearing = false;

                    if(wstat == 0){
                        if(*wrt_log) printf("running");
                    } else {
                        if(WIFEXITED(status)) {
                            if(!*wrt_log) printf("[MSG]\t%d: id= %d ", i, pids[i]);
                            printf("exit, status: %d\n", WEXITSTATUS(status));
                            is_clearing = true;
                        } else if(WIFSIGNALED(status)) {
                            if(!*wrt_log) printf("[MSG]\t%d: id= %d ", i, pids[i]);
                            printf("killed by signal %d\n", WIFSIGNALED(status));
                            is_clearing = true;
                        } else if(WIFSTOPPED(status)) {
                            if(*wrt_log) printf("stoped by signal %d", WSTOPSIG(status));
                        } else if(WIFCONTINUED(status)) {
                            if(*wrt_log) printf("continued");
                        }
                    }

                    if(is_clearing) {
                        pids[i] = 0;
                        n_p --;
                    }

                    if(*wrt_log) printf("\n");
                }
            }

            if(msg[0] == MSG_SNT && msg[1] == MSG_LAUNCH) {
                msg[0] = MSG_CLR;
                msg[1] = MSG_CLR;

                if(n_p == MAX_N_P) {
                    printf("ERROR processes limit\n");
                    continue;
                }

                printf("buffer: '%s'\n", buffer);

                read_pinfo(buffer, pinfo);

                p_id = fork();

                if(p_id == 0) {
                    break; //children
                }


                printf("[MSG]\tlaunch pid: %d\n", p_id);

                for(int i = 0; i < MAX_N_P; i ++) {
                    if(pids[i] == 0) {
                        pids[i] = p_id;
                        break;
                    }
                }

                n_p ++;
            }
        }

        if(p_id == 0) {
            execvpe(pinfo->args[0], pinfo->args, envp);
            printf("[ERROR]\tlaunch failed: %s\n", pinfo->args[0]);
        }

        exit(0);
    }

    //user interface

    while(!*is_exit) {
        buffer = fgets(buffer, MAX_LEN, stdin);

        int buffer_len = strlen(buffer);
        buffer[buffer_len - 1] = '\0';
        buffer_len --;

        if(strcmp(buffer, "end") == 0) {
            *is_exit = true;
            continue;
        }
        if(strcmp(buffer, "log") == 0) {
            *wrt_log = true;
            continue;
        }
        if(strcmp(buffer, "stop") == 0) {
            *wrt_log = false;
            continue;
        }

        while(msg[0] != MSG_CLR) {
        }
        msg[0] = MSG_WRG;

        msg[1] = MSG_LAUNCH;
        msg[0] = MSG_SNT;
    }

    if(p_id == 0) {

        printf("start\n");
        sleep(5);
        printf("end\n");

        exit(0);
    }

    if(p_id != 0) {
        detach_shmem(shmem);
        detach_shmem(shmem + 1);
        detach_shmem(shmem + 2);
        detach_shmem(shmem + 3);
    }

    /*

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
    */
    exit(0);
    return 0;
}



