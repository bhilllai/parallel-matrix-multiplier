#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<string.h>
#include<pthread.h>
#include<time.h>

int I, J, K;

long long int *input1;
long long int *input2;

long long int *output;

int NO_OF_THREADS;

pthread_mutex_t mutex_lock;

int numberOfMult = 0;

int row_to_mult = 0;
int col_to_mult = 0;

int isRow = 0;

char *output_file_name;

void multiplyRowCol(int row, int col) {
    long long int res = 0;

    for(int j=0; j<J; j++) {
        res += (input1[row*J + j] * input2[col*J + j]);
    }

    output[row*K + col] = res;
}

void* threadAllocator(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex_lock);
        int i = (row_to_mult)%I;
        int k = (col_to_mult)%K;

        long long int check = output[i*K + k];
        long long int checkRow = input1[i*J + (J-1)];
        long long int checkCol = input2[k*J + (J-1)];

        if(check == -1 && checkRow != -1 && checkCol != -1) {
            numberOfMult++;
            output[i*K + k] = 0;
        }

        if(isRow == 0) {
            row_to_mult = (row_to_mult+1)%I;
            isRow = 1;
        }
        else {
            col_to_mult = (col_to_mult+1)%K;
            if(col_to_mult == 0) {
                isRow = 0;
            }
        }

        int row = i;
        int col = k;

        pthread_mutex_unlock(&mutex_lock);

        if(check == -1 && checkRow != -1 && checkCol != -1) {
            multiplyRowCol(row, col);
        }

        if(numberOfMult >= I*K) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    
    I = atoi(argv[1]);
    J = atoi(argv[2]);
    K = atoi(argv[3]);

    NO_OF_THREADS = atoi(argv[4]);

    output_file_name = argv[5];

    unsigned long long int output_size = (unsigned long long int) I * (unsigned long long int) K;

    output = (long long int *) malloc((output_size) * (sizeof(long long int)));

    key_t shm_key[3];

    shm_key[0] = ftok("/", 'A');
    shm_key[1] = ftok("/", 'B');
    shm_key[2] = ftok("/", '2');

    int shm_id[3];

    while((shm_id[0] = shmget(shm_key[0], sizeof(long long int [I][J]), 0666)) == -1);
    while((shm_id[1] = shmget(shm_key[1], sizeof(long long int [K][J]), 0666)) == -1);
    shm_id[2] = shmget(shm_key[2], sizeof(int), 0666);

    input1 = (long long int *) shmat(shm_id[0], 0, 0);
    input2 = (long long int *) shmat(shm_id[1], 0, 0);
    int *flag;
    flag = (int *) shmat(shm_id[2], 0, 0);

    pthread_t t_id[NO_OF_THREADS];

    for(int i=0; i<I; i++) {
        for(int k=0; k<K; k++) {
            output[i*K + k] = -1;
        }
    }
 
    FILE *p2_time_file = fopen("P2_TIME_DATA.txt", "a");

    fprintf(p2_time_file, "%d,", NO_OF_THREADS);

    numberOfMult = 0;
    row_to_mult = 0;
    col_to_mult = 0;

    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);

    pthread_mutex_init(&mutex_lock, NULL);

    for(int t_ind = 0; t_ind < NO_OF_THREADS; t_ind++) {
        pthread_create(&t_id[t_ind], NULL, &threadAllocator, NULL);
    }

    for(int t_ind = 0; t_ind < NO_OF_THREADS; t_ind++) {
        pthread_join(t_id[t_ind], NULL);
    }

    pthread_mutex_destroy(&mutex_lock);

    clock_gettime(CLOCK_REALTIME, &endTime);

    long long int secondsElapsed = (endTime.tv_sec - startTime.tv_sec)*1e9; 
    long long int nanoSecondsElapsed = endTime.tv_nsec - startTime.tv_nsec;

    fprintf(p2_time_file, "%lld\n", secondsElapsed + nanoSecondsElapsed);

    fclose(p2_time_file);

    FILE *output_file_ptr = fopen(output_file_name, "w");

    for(int i=0; i<I; i++) {
        for(int k=0; k<K; k++) {
            fprintf(output_file_ptr, "%lld ", output[i*K + k]);
        }
        fprintf(output_file_ptr, "\n");
    }

    fclose(output_file_ptr);

    free(output);

    shmdt((void *) input1);
    shmdt((void *) input2);

    shmctl(shm_id[0], IPC_RMID, NULL);
    shmctl(shm_id[1], IPC_RMID, NULL);

    flag[0] = 1;

    return 0;
}
