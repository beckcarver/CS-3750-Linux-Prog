
/*
* mmult.c
*
* COSC 3750, Homework 11
*
* This is the code for the mmult program. It multiplies
* two matrices with the given nummber of threads and
* stores the result in an output file.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct thread_data {
    int row, column;
    int rowsA, colsA, rowsB, colsB, rowsC, colsC;
    int threadsFinished;
    double *A;
    double *B;
    double *C;
    pthread_mutex_t *mutex1, *mutex2;
    pthread_cond_t *cond;
};

void* multiply(void *arg)
{
  //clock_t start, end;
  struct thread_data *data = (struct thread_data *) arg;
  int r, c;
  /*
      printf("wait\n");

  pthread_mutex_lock(data->mutex2);
  pthread_cond_signal(data->cond);
  pthread_cond_wait(data->cond, data->mutex2);
  pthread_mutex_unlock(data->mutex2);
        printf("go\n");
*/
  while(1){
    //get row and column
    pthread_mutex_lock(data->mutex1);

    r = data->row;
    c = data->column++;
    if(c == data->colsB){
      c = 0;
      data->column = 1;
      r++;
      data->row++;
    }
    if(r == data->rowsA){
      pthread_mutex_unlock(data->mutex1);
      break;
    }

    pthread_mutex_unlock(data->mutex1);

    //compute dot product
    double sum = 0;
    for (int i = 0; i < data->colsA; i++) {
      sum += data->A[r*data->colsA + i] * data->B[i*data->colsB + c];
    }



    //pthread_mutex_lock(data->mutex2);
    data->C[r*data->colsB + c] = sum;
    //pthread_cond_signal(data->cond);
    //pthread_mutex_unlock(data->mutex2);

  }
  pthread_mutex_lock(data->mutex2);
  data->threadsFinished++;
  pthread_cond_signal(data->cond);
  pthread_mutex_unlock(data->mutex2);
  pthread_exit(NULL);
}

int main(int argc, char** argv){
  int nthreads;
  int rows1, cols1, rows2, cols2;
  double *A, *B, *C;
  FILE *inFile1, *inFile2, *outFile;
  struct timespec start, end;
  double time;
  //check args
  if (argc == 4) {
    nthreads = 1;
  }
  else if (argc == 5) {
    nthreads = atoi(argv[4]);
    if (nthreads <= 0) {
      fprintf(stderr, "Error: Invalid number of threads.\n");
      exit(1);
    }
  }
  else {
    fprintf(stderr, "Usage: mmult matrix1 matrix2 result [nthreads]\n");
    exit(1);
  }

  //open files
  inFile1 = fopen(argv[1], "r");
  inFile2 = fopen(argv[2], "r");
  outFile = fopen(argv[3], "w+");
  if (inFile1 == NULL || inFile2 == NULL || outFile == NULL) {
    fprintf(stderr, "Error: Could not open input/output files.\n");
    exit(1);
  }

  //check dimensions
  if(fread(&rows1, sizeof(int), 1, inFile1) != 1 ||
     fread(&cols1, sizeof(int), 1, inFile1) != 1 ||
     fread(&rows2, sizeof(int), 1, inFile2) != 1 ||
     fread(&cols2, sizeof(int), 1, inFile2) != 1){

    fprintf(stderr, "Error reading input files.\n");
    exit(1);
  }

  if(cols1 != rows2 || rows1 <= 0 || cols1 <= 0 || rows2 <= 0 || cols2 <= 0){
    fprintf(stderr, "Error: Invalid matrix multiplication");
    exit(1);
  }

  //load in matrices
  A = malloc(rows1 * cols1 * sizeof(double));
  B = malloc(rows2 * cols2 * sizeof(double));
  C = malloc(rows1 * cols2 * sizeof(double));
  if(A == NULL || B == NULL || C == NULL){
    fprintf(stderr, "Error allocating memory for matrices.\n");
    exit(1);
  }

  if(fread(A, sizeof(double), rows1 * cols1, inFile1) != rows1 * cols1 ||
     fread(B, sizeof(double), rows2 * cols2, inFile2) != rows2 * cols2){

    fprintf(stderr, "Error reading martix data.\n");
    exit(1);
  }
 
  if(ferror(inFile1) || ferror(inFile2)){
    fprintf(stderr, "Error reading matrix file.\n");
    exit(1);
  }
  if(nthreads == 1){//normal computation
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < rows1; i++){
      for(int j = 0; j < cols2; j++){
        for(int k = 0; k < cols1; k++){
          C[i * cols2 + j] += A[i * cols1 + k] * B[k * cols2 + j];
        }
      }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    if(fwrite(&rows1, sizeof(int), 1, outFile) != 1 ||
       fwrite(&cols2, sizeof(int), 1, outFile) != 1 ||
       fwrite(C, sizeof(double), rows1*cols2, outFile) != rows1*cols2){

      fprintf(stderr, "Error writing matrix data to file.\n");
      exit(1);
    }
  }
  else{//using threads
  printf("threads: %d\n", nthreads);
    int rtn;
    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    //initialize struct to be accessed by threads
    struct thread_data tData;
    tData.threadsFinished = 0;
    tData.row = 0;
    tData.column = 0;
    tData.rowsA = rows1;
    tData.colsA = cols1;
    tData.rowsB = rows2;
    tData.colsB = cols2;
    tData.rowsC = 0;
    tData.colsC = cols2;
    tData.A = A;
    tData.B = B;
    tData.C = C;
    tData.mutex1 = &mutex1;
    tData.mutex2 = &mutex2;
    tData.cond = &cond;

    if(fwrite(&rows1, sizeof(int), 1, outFile) != 1 ||
       fwrite(&cols2, sizeof(int), 1, outFile) != 1){

      fprintf(stderr, "Error writing matrix data to file.\n");
      exit(1);
    }

    pthread_t threads[nthreads];
    //create threads
    pthread_mutex_lock(&mutex1);
    for(int i = 0; i < nthreads; i++) {
      rtn=pthread_create(&threads[i],NULL,multiply,&tData);
      if(rtn) {
        fprintf(stderr,"Error creating pthread.\n");
        return 1;
      }
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_unlock(&mutex1);


    pthread_mutex_lock(&mutex2);
    while (tData.threadsFinished < nthreads) {
      pthread_cond_wait(&cond, &mutex2);
    }
    pthread_mutex_unlock(&mutex2);

    clock_gettime(CLOCK_MONOTONIC, &end);

    //join threads
    for(int i = 0; i < nthreads; i++) {
      rtn=pthread_join(threads[i],NULL);
      if(rtn) {
        fprintf(stderr,"Error joining pthreads\n");
        return 1;
      }
    }

    if(fwrite(C, sizeof(double), rows1 * cols2  ,outFile) != rows1 * cols2){
      fprintf(stderr, "Error writing matrix data to file.\n");
      exit(1);
    }
  }
  free(A);
  free(B);
  free(C);
  fclose(inFile1);
  fclose(inFile2);
  fclose(outFile);

  printf("Matrix sizes:\n\tM: %d\n\tN: %d\n\tP: %d\n", rows1, cols1, cols2);
  printf("Worker threads: %d\n", nthreads);
  time = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
  printf("Total time: %f seconds\n", time);
  return 0;
}