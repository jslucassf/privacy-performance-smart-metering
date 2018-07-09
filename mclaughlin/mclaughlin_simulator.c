#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int N;
int *measurements;
int *load_leveled_measurements;
double SM_time = 0;
double AG_time = 0;

struct meter_info{
  int id;
};

void init_variables(int N_value) {
  N = N_value;
  srand(time(NULL));
  measurements = calloc(N, sizeof(int));
  load_leveled_measurements = calloc(N, sizeof(int));
  int i;
  for (i = 0; i < N; i++) {
    measurements[i] = rand() % 8000;
  }
}

void meter_load_leveling(struct meter_info *info) {
  clock_t begin = clock();
  int m = measurements[info->id]; //net demand from all apliances in the house
  int c = 4000; //fictional battery state of charge
  int b = c - m; //battery rate of charging
  int u = m + b; //measurement disclosed to utility
  load_leveled_measurements[info->id] = u;
  clock_t end = clock();
  SM_time += (double)(end - begin) / CLOCKS_PER_SEC;
  free(info);
}

void step_1() {
  //printf("Meters are adding the battery charging rate...\n\n", N);

  int i;
  for (i = 0; i < N; i++) {
    struct meter_info *info = malloc(sizeof(struct meter_info));
    info->id = i;
    meter_load_leveling(info);
  }
}

void step_2() {
  //printf("Power provider is summing all load leveled measurements to obtain the total (M)\n\n");
  clock_t begin = clock();
  int i;
  int M = 0;
  for (i = 0; i < N; i++) {
    M += load_leveled_measurements[i];
  }
  clock_t end = clock();
  AG_time += (double)(end - begin) / CLOCKS_PER_SEC;

  //printf("Total consumption regional consumption is M = %d\n", M, M_real);
  printf("%f %f\n", SM_time / N, AG_time);
}

int main(int argc, char *argv[]) {
  init_variables(atoi(argv[1]));
  
  step_1();
  step_2();
}
