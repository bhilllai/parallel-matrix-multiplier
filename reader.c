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

long long int *in_file1;
long long int *in_file2;

long long int *input1;
long long int *input2;

int readLine1 = -1;
int readLine2 = -1;

int NO_OF_THREADS;

int isInput1 = 0;

char* input_file_name1;
char* input_file_name2;

pthread_mutex_t mutex_lock;

void transposeFile() {
    FILE *in_file_ptr2 = fopen(input_file_name2, "r");

    int count = 0;

    long long int num = 0;

    long long int transpose[K][J];

    while(fscanf(in_file_ptr2, "%lld", &num) != EOF) {
        transpose[count%K][count/K] = num;
        count++;
    }

    fclose(in_file_ptr2);

    FILE *in_file_ptr3 = fopen("transpose.txt", "w");

    for(int i=0; i<K; i++) {
        for(int j=0; j<J; j++) {
            fprintf(in_file_ptr3, "%lld ", transpose[i][j]);
        }
        fprintf(in_file_ptr3, "\n");
    }

    fclose(in_file_ptr3);
}

void getFilePointers() {
    FILE *in_file_ptr1 = fopen(input_file_name1, "r");

    int count = 0;

    long long int num = 0;

    while(count < I*J) {
        if(count%J == 0) {
            in_file1[count/J] = ftell(in_file_ptr1);
        }
        fscanf(in_file_ptr1, "%lld", &num);
        count++;
    }

    fclose(in_file_ptr1);

    FILE *in_file_ptr2 = fopen("transpose.txt", "r");

    count = 0;

    num = 0;

    while(count < K*J) {
        if(count%J == 0) {
            in_file2[count/J] = ftell(in_file_ptr2);
        }
        fscanf(in_file_ptr2, "%lld", &num);
        count++;
    }

    fclose(in_file_ptr2);
}

void readerInput1(int lineNum) {
    FILE *file_ptr = fopen(input_file_name1, "r");
    fseek(file_ptr, in_file1[lineNum], SEEK_SET);

    long long int num = 0;

    for(int j=0; j<J; j++) {
        fscanf(file_ptr, "%lld", &num);
        input1[lineNum*J + j] = num;
    }

    fclose(file_ptr);
}

void readerInput2(int lineNum) {
    FILE *file_ptr = fopen("transpose.txt", "r");
    fseek(file_ptr, in_file2[lineNum], SEEK_SET);

    long long int num = 0;

    for(int j=0; j<J; j++) {
        fscanf(file_ptr, "%lld", &num);
        input2[lineNum*J + j]= num;
    }

    fclose(file_ptr);
}

void* threadAllocator(void* argument) {
    while(1) {
        pthread_mutex_lock(&mutex_lock);
        if(isInput1 == 0) {
            isInput1 = 1;
            if(readLine1 < I) {
                readLine1++;
            }
            int lineNum = readLine1;
            pthread_mutex_unlock(&mutex_lock);

            if(lineNum < I) {
                readerInput1(lineNum);
            }
        }
        else {
            isInput1 = 0;
            if(readLine2 < K) {
                readLine2++;
            }
            int lineNum = readLine2;
            pthread_mutex_unlock(&mutex_lock);

            if(lineNum < K) {
                readerInput2(lineNum);
            }
        }

        if(readLine1 == I && readLine2 == K) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {

    I = atoi(argv[1]);
    J = atoi(argv[2]);
    K = atoi(argv[3]);

    NO_OF_THREADS = atoi(argv[4]);

    input_file_name1 = argv[5];
    input_file_name2 = argv[6];

    transposeFile();

    in_file1 = (long long int *) malloc(I *sizeof(long long int));
    in_file2 = (long long int *) malloc(K *sizeof(long long int));

    getFilePointers();

    key_t shm_key[3];

    shm_key[0] = ftok("/", 'A');
    shm_key[1] = ftok("/", 'B');
    shm_key[2] = ftok("/", '1');

    int shm_id[3];

    shm_id[0] = shmget(shm_key[0], sizeof(long long int [I][J]), 0666 | IPC_CREAT);
    shm_id[1] = shmget(shm_key[1], sizeof(long long int [K][J]), 0666 | IPC_CREAT);
    shm_id[2] = shmget(shm_key[2], sizeof(int), 0666 | IPC_CREAT);

    input1 = (long long int *) shmat(shm_id[0], 0, 0);
    input2 = (long long int *) shmat(shm_id[1], 0, 0);
    int *flag = (int *) shmat(shm_id[2], 0, 0);

    pthread_t t_id[NO_OF_THREADS];

    for(int i=0; i<I; i++) {
        for(int j=0; j<J; j++) {
            input1[i*J + j] = -1;
        }
    }

    for(int k=0; k<K; k++) {
        for(int j=0; j<J; j++) {
            input2[k*J + j] = -1;
        }
    }

    FILE *p1_time_file = fopen("P1_TIME_DATA.txt", "a");

    fprintf(p1_time_file, "%d,", NO_OF_THREADS);

    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);

    pthread_mutex_init(&mutex_lock, NULL);

    readLine1 = -1;
    readLine2 = -1;
    isInput1 = 0;
    
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

    fprintf(p1_time_file, "%lld\n", secondsElapsed + nanoSecondsElapsed);

    fclose(p1_time_file);

    free(in_file1);
    free(in_file2);

    shmdt((void *) input1);
    shmdt((void *) input2);

    flag[0] = 1;

    return 0;
}
