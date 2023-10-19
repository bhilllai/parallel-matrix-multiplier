#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<sys/wait.h>
#include<string.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>
#include<signal.h>

int I, J, K;

int processExecute1 = 1;

int time_quantum = 2;

int main(int argc, char *argv[]) {

    I = atoi(argv[1]);
    J = atoi(argv[2]);
    K = atoi(argv[3]);

    key_t shm_key[2];

    shm_key[0] = ftok("/", '1');
    shm_key[1] = ftok("/", '2');

    int shm_id[2];

    shm_id[0] = shmget(shm_key[0], sizeof(int), 0666 | IPC_CREAT);
    shm_id[1] = shmget(shm_key[1], sizeof(int), 0666 | IPC_CREAT);

    int *flag1 = (int *) shmat(shm_id[0], 0, 0);
    int *flag2 = (int *) shmat(shm_id[1], 0, 0);

    *flag1 = 0;
    *flag2 = 0;

    pid_t cpid1, cpid2;

    cpid1 = fork();

    struct timespec startTimeTurnaroundP1;
    struct timespec endTimeTurnaroundP1;

    struct timespec startTimeTurnaroundP2;
    struct timespec endTimeTurnaroundP2;

    long long int waitingTimeP1 = 0;
    long long int waitingTimeP2 = 0;

    FILE *s_time_file = fopen("S_TIME_DATA.txt", "a");

    // Reference: https://github.com/VenomCJ7/Scheduler_Uniprocessor/blob/main/P3_RR.c

    if(cpid1 < 0) {
        perror("fork1 error");
    }
    else if(cpid1 == 0) {
        execlp("./P1.out", "P1.out", argv[1], argv[2], argv[3], "200", argv[4], argv[5], NULL);
    }
    else {
        clock_gettime(CLOCK_REALTIME, &startTimeTurnaroundP1);
        cpid2 = fork();

        if(cpid2 < 0) {
            perror("fork2 error");
        }
        else if(cpid2 == 0) {
            execlp("./P2.out", "P2.out", argv[1], argv[2], argv[3], "200", argv[6], NULL);
        }
        else {
            clock_gettime(CLOCK_REALTIME, &startTimeTurnaroundP2);

            kill(cpid1, SIGSTOP);
            kill(cpid2, SIGSTOP);

            while(*flag1 == 0 && *flag2 == 0) {
                struct timespec startTime;
                struct timespec endTime;

                if(processExecute1 == 1) {
                    printf("Process 1 \n");
                    clock_gettime(CLOCK_REALTIME, &startTime);
                    kill(cpid1, SIGCONT);
                    usleep(time_quantum * 1000);
                    kill(cpid1, SIGSTOP);
                    clock_gettime(CLOCK_REALTIME, &endTime);

                    long long int secondsElapsed = (endTime.tv_sec - startTime.tv_sec)*1e9; 
                    long long int nanoSecondsElapsed = endTime.tv_nsec - startTime.tv_nsec;

                    waitingTimeP2 += secondsElapsed + nanoSecondsElapsed;

                    processExecute1 = 2;
                }
                else if(processExecute1 == 2) {
                    printf("Process 2 \n");
                    clock_gettime(CLOCK_REALTIME, &startTime);
                    kill(cpid2, SIGCONT);
                    usleep(time_quantum * 1000);
                    kill(cpid2, SIGSTOP);
                    clock_gettime(CLOCK_REALTIME, &endTime);

                    long long int secondsElapsed = (endTime.tv_sec - startTime.tv_sec)*1e9; 
                    long long int nanoSecondsElapsed = endTime.tv_nsec - startTime.tv_nsec;

                    waitingTimeP1 += secondsElapsed + nanoSecondsElapsed;

                    processExecute1 = 1;
                }
            }

            int endOver1 = 0, endOver2 = 0;

            if(*flag1 == 1) {
                clock_gettime(CLOCK_REALTIME, &endTimeTurnaroundP1);
                endOver1 = 1;
            }
            if(*flag2 == 1) {
                clock_gettime(CLOCK_REALTIME, &endTimeTurnaroundP2);
                endOver2 = 1;
            }

            while(*flag1 == 0) {
                printf("Process 1 \n");
                kill(cpid1, SIGCONT);
                usleep(time_quantum * 1000);
                kill(cpid1, SIGSTOP);
            }

            while(*flag2 == 0) {
                printf("Process 2 \n");
                kill(cpid2, SIGCONT);
                usleep(time_quantum * 1000);
                kill(cpid2, SIGSTOP);
            }

            if(endOver1 == 0) {
                clock_gettime(CLOCK_REALTIME, &endTimeTurnaroundP1);
                endOver1 = 1;
            }
            if(endOver2 == 0) {
                clock_gettime(CLOCK_REALTIME, &endTimeTurnaroundP2);
                endOver2 = 1;
            }
        }
    }

    fprintf(s_time_file,"%s,%s,%s,", argv[1], argv[2], argv[3]);

    long long int secondsElapsed = (endTimeTurnaroundP1.tv_sec - startTimeTurnaroundP1.tv_sec)*1e9; 
    long long int nanoSecondsElapsed = endTimeTurnaroundP1.tv_nsec - startTimeTurnaroundP1.tv_nsec;

    fprintf(s_time_file, "%lld,", secondsElapsed + nanoSecondsElapsed);

    secondsElapsed = (endTimeTurnaroundP2.tv_sec - startTimeTurnaroundP2.tv_sec)*1e9; 
    nanoSecondsElapsed = endTimeTurnaroundP2.tv_nsec - startTimeTurnaroundP2.tv_nsec;

    fprintf(s_time_file, "%lld,", secondsElapsed + nanoSecondsElapsed);

    fprintf(s_time_file, "%lld,", waitingTimeP1);
    fprintf(s_time_file, "%lld\n", waitingTimeP2);

    fclose(s_time_file);

    shmctl(shm_id[0], IPC_RMID, NULL);
    shmctl(shm_id[1], IPC_RMID, NULL);

    return 0;
}
