#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int N;
int *measurements;
int *masked_measurements;
double SM_time = 0;
double AG_time = 0;

struct meter_info{
  int id;
};

void init_variables(int N_value) {
  N = N_value;
  srand(time(NULL));
  measurements = calloc(N, sizeof(int));
  masked_measurements = calloc(N, sizeof(int));
  int i;
  for (i = 0; i < N; i++) {
    measurements[i] = rand() % 8000;
  }
}

void meter_add_noise(struct meter_info *info) {
  clock_t begin = clock();
  int m = measurements[info->id];
  float ea = (m * N) * 0.05; // 10 min granularity
  int X = (0.726 * ea) / sqrt((double) N);
  int x = rand() % (2 * X) - (X);
  masked_measurements[info->id] = m + x;
  clock_t end = clock();
  SM_time += (double)(end - begin) / CLOCKS_PER_SEC;
  free(info);
}

void step_1() {
  //printf("Meters are adding noise...\n\n", N);

  int i;
  for (i = 0; i < N; i++) {
    struct meter_info *info = malloc(sizeof(struct meter_info));
    info->id = i;
    meter_add_noise(info);
  }
}

void step_2() {
  //printf("Power provider is summing all masked measurements to obtain the total (M)\n\n");
  clock_t begin = clock();
  int i;
  int M = 0;
  int M_real = 0;
  for (i = 0; i < N; i++) {
    M += masked_measurements[i];
  }
  clock_t end = clock();
  AG_time += (double)(end - begin) / CLOCKS_PER_SEC;
  for (i = 0; i < N; i++) {
    M_real += measurements[i];
  }

  //printf("Total consumption regional consumption is M = %d (real is %d)\n", M, M_real);
  printf("%f %f\n", SM_time / N, AG_time);
}

int main(int argc, char *argv[]) {
  init_variables(atoi(argv[1]));
  
  step_1();
  step_2();
}
