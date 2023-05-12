/*
 * Beckham Carver
 * mmult.c
 *
 * COSC 3750, Homework 11
 *
 * Code parralelizes matrix multiplication
 * using pthreads and writes final binary
 * to file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct ThreadData {
    int row, column;
    int rowsA, colsA, rowsB, colsB, rowsC, colsC;
    int threadsLeft;
    double *A;
    double *B;
    double *C;
    pthread_mutex_t *mutex1, *mutex2;
    pthread_cond_t *cond;
};



void* multiply(void *arg);

int main(int argc, char** argv){
    FILE *inFile1, *inFile2, *outFile;
    struct timespec start, end;
    double time;
    struct ThreadData t_data;

    //check args
    if (argc == 4) {
        t_data.threadsLeft = 1;
    }
    else if (argc == 5) {
        t_data.threadsLeft = atoi(argv[4]);
        if (t_data.threadsLeft <= 0) {
            printf("Error: Invalid number of threads given.\n");
            exit(1);
        }
    }
    else {
        printf("Usage: ./mmult matrix1 matrix2 result_file num_threads\n");
        exit(1);
    }

    // open and check files
    inFile1 = fopen(argv[1], "r");
    inFile2 = fopen(argv[2], "r");
    outFile = fopen(argv[3], "w+");
    if (inFile1 == NULL || inFile2 == NULL || outFile == NULL) {
        printf("Error: Could not open given input/output files.\n");
        exit(1);
    }

    // verify dimensions
    if (fread(&t_data.rowsA, sizeof(int), 1, inFile1) != 1 ||
        fread(&t_data.colsA, sizeof(int), 1, inFile1) != 1 ||
        fread(&t_data.rowsB, sizeof(int), 1, inFile2) != 1 ||
        fread(&t_data.colsB, sizeof(int), 1, inFile2) != 1){

        fprintf(stderr, "Error: could not open given filenames.\n");
        exit(1);
    }

    // check if valid for computation
    if(t_data.colsA != t_data.rowsB){
        printf("Error: Invalid matrices, they do not share col/row lengths");
        exit(1);
    }

    // allocate for and read in matrices
    t_data.A = malloc(t_data.rowsA * t_data.colsA * sizeof(double));
    t_data.B = malloc(t_data.rowsB * t_data.colsB * sizeof(double));
    t_data.C = malloc(t_data.rowsA * t_data.colsB * sizeof(double));
    if(t_data.A == NULL || t_data.B == NULL || t_data.C == NULL){
        printf("Error: Memory allocation failed.\n");
        exit(1);
    }
    if(fread(t_data.A, sizeof(double), t_data.rowsA * t_data.colsA, inFile1) != t_data.rowsA * t_data.colsA ||
            fread(t_data.B, sizeof(double), t_data.rowsB * t_data.colsB, inFile2) != t_data.rowsB * t_data.colsB){

        printf("Error: Matrix data invalid.\n");
        exit(1);
    }

    if(ferror(inFile1) || ferror(inFile2)){
        printf("Error: file read failed.\n");
        exit(1);
    }


    // if single threaded, standard computation
    if(t_data.threadsLeft == 1){
        clock_gettime(CLOCK_MONOTONIC, &start);
        for(int i = 0; i < t_data.rowsA; i++){
            for(int j = 0; j < t_data.colsB; j++){
                for(int k = 0; k < t_data.colsB; k++){
                    t_data.C[i * t_data.colsB + j] += 
                    t_data.A[i * t_data.colsA + k] * 
                    t_data.B[k * t_data.colsB + j];
                }
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        if (fwrite(&t_data.rowsA, sizeof(int), 1, outFile) != 1 ||
            fwrite(&t_data.colsB, sizeof(int), 1, outFile) != 1 ||
            fwrite(t_data.C, sizeof(double), t_data.rowsA*t_data.colsB, outFile) 
            != t_data.rowsA*t_data.colsB) {

            printf("Error: Failed to write matrix data to file.\n");
            exit(1);
        }
    }
    // multi-threaded computation
    else{
        printf("threads: %d\n", t_data.threadsLeft);
        int rtn;
        pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

        t_data.mutex1 = &mutex1;
        t_data.mutex2 = &mutex2;
        t_data.cond = &cond;
        t_data.row = 0;
        t_data.column = 0;
        t_data.rowsC = 0;

        if(fwrite(&t_data.rowsA, sizeof(int), 1, outFile) != 1 ||
                fwrite(&t_data.colsB, sizeof(int), 1, outFile) != 1){

            printf("Error: Failed to write to file.\n");
            exit(1);
        }

        pthread_t threads[t_data.threadsLeft];
        //create threads
        int createdThreads = t_data.threadsLeft;
        pthread_mutex_lock(&mutex1);
        for(int i = 0; i < createdThreads; i++) {
            rtn=pthread_create(&threads[i],NULL,multiply,&t_data);
            if(rtn) {
                printf("Error: Failed creating pthread.\n");
                return 1;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_unlock(&mutex1);


        pthread_mutex_lock(&mutex2);
        while (t_data.threadsLeft > 0) {
            pthread_cond_wait(&cond, &mutex2);
        }
        pthread_mutex_unlock(&mutex2);

        clock_gettime(CLOCK_MONOTONIC, &end);

        //join threads
        for(int i = 0; i < createdThreads; i++) {
            rtn=pthread_join(threads[i],NULL);
            if(rtn) {
                printf("Error: Failed to join pthreads.\n");
                return 1;
            }
        }
        t_data.threadsLeft = createdThreads;

        if(fwrite(t_data.C, sizeof(double), t_data.rowsA * t_data.colsB  ,outFile) != t_data.rowsA * t_data.colsB){
            printf("Error: Failed writing to file.\n");
            exit(1);
        }
    }
    fclose(inFile1);
    fclose(inFile2);
    fclose(outFile);

    printf("Matrix sizes:\n\tM: %d\n\tN: %d\n\tP: %d\n", t_data.rowsA, t_data.colsA, t_data.colsB);
    printf("Worker threads: %d\n", t_data.threadsLeft);
    time = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
    printf("Total time: %f seconds\n", time);
    return 0;
}

void* multiply(void *arg)
{
    // ew gross 'c' cast 
    struct ThreadData *data = (struct ThreadData *) arg;
    int row, col;

    while(1) {
        // lock and retrieve current column/row
        pthread_mutex_lock(data->mutex1);
        row = data->row;
        col = data->column++; //always increment for next thread
        if(col == data->colsB){
            col = 0;
            data->column = 1;
            row++;
            data->row++;
        }
        
        pthread_mutex_unlock(data->mutex1);
        if(row == data->rowsA){break;}

        //compute dot product
        double sum = 0;
        for (int i = 0; i < data->colsA; i++) {
            sum += data->A[row * data->colsA + i] * data->B[i*data->colsB + col];
        }

        data->C[row*data->colsB + col] = sum;
    }
    pthread_mutex_lock(data->mutex2);
    data->threadsLeft--;
    pthread_cond_signal(data->cond);
    pthread_mutex_unlock(data->mutex2);
    pthread_exit(NULL);
}
