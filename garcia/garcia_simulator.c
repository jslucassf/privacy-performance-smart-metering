#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gmp.h"
#include "paillier.h"

int N;
int *measurements;
double SM_time = 0;
double AG_time = 0;

int *self_secrets;
int *partial_Ms;

paillier_ciphertext_t ***enc_secrets;
paillier_ciphertext_t **partial_ms;
paillier_pubkey_t **pubkeys;
paillier_prvkey_t **prvkeys;

struct meter_info{
  int id;
};

void init_variables(int N_value) {
  N = N_value;
  srand(time(NULL));
  measurements = calloc(N, sizeof(int));
  self_secrets = calloc(N, sizeof(int));
  enc_secrets = calloc(N, sizeof(paillier_ciphertext_t **));
  int i;
  for (i = 0; i < N; i++) {
    measurements[i] = rand() % 8000;
    enc_secrets[i] = calloc(N, sizeof(paillier_ciphertext_t *));
  }
  partial_ms = calloc(N, sizeof(paillier_ciphertext_t *));
  partial_Ms = calloc(N, sizeof(int *));

  pubkeys = calloc(N, sizeof(paillier_pubkey_t));
  prvkeys = calloc(N, sizeof(paillier_pubkey_t));
}

void meter_start(struct meter_info *info) {
  paillier_pubkey_t *pubkey;
  paillier_prvkey_t *prvkey;
  paillier_keygen(2048, &pubkey, &prvkey, paillier_get_rand_devurandom);

  pubkeys[info->id] = pubkey;
  prvkeys[info->id] = prvkey;
}

void meter_secret_sharings(struct meter_info *info) {
  clock_t begin = clock();
  int m = measurements[info->id];
  
  int j;
  int sum_secrets = 0;
  for (j = 0; j < N; j++) {
    
    int sij;
    if (j != N - 1) {
      sij = rand() % RAND_MAX - (RAND_MAX / 2);
      sum_secrets += sij;
    } else {
      sij = m - sum_secrets;
    }

    if (j == info->id) {
      self_secrets[info->id] = sij;
    } else {
      paillier_plaintext_t* pt = paillier_plaintext_from_ui(sij);
      enc_secrets[j][info->id] = paillier_enc(NULL, pubkeys[j], pt, 
          paillier_get_rand_devurandom);
    }
  }
  clock_t end = clock();
  SM_time += (double)(end - begin) / CLOCKS_PER_SEC;
 
  free(info);
}

void meter_add_self_secret(struct meter_info *info) {
  clock_t begin = clock();
  paillier_plaintext_t* pt = paillier_dec(NULL, pubkeys[info->id], 
      prvkeys[info->id], partial_ms[info->id]);

  partial_Ms[info->id] = (int) mpz_get_ui(pt->m) + self_secrets[info->id];
  clock_t end = clock();
  SM_time += (double)(end - begin) / CLOCKS_PER_SEC;
  free(info);
}



void steps_1_and_2() {
  //printf("Initializing %d meters and generating keys...\n\n", N);

  int i;
  for (i = 0; i < N; i++) {
    struct meter_info *info = malloc(sizeof(struct meter_info));
    info->id = i;
    meter_start(info);
    free(info);
  }
}

void steps_3_and_4() {
  //printf("Meters are generating secret sharings and encrypting them...\n\n");

  int i;
  for (i = 0; i < N; i++) {
    struct meter_info *info = malloc(sizeof(struct meter_info));
    info->id = i;
    meter_secret_sharings(info);
  }
}

void step_5() {
  //printf("Power provider is multiplying the shares that has been "
  //    "encrypted with the same public key\n\n");
  clock_t begin = clock();
  int i, j;
  for (i = 0; i < N; i++) {
    paillier_ciphertext_t* res = paillier_create_enc_zero();
    for (j = 0; j < N; j++) {
      if (i != j) {
        paillier_mul(pubkeys[i], res, res, enc_secrets[i][j]);
      }
    }
    partial_ms[i] = res;
  }

  clock_t end = clock();
  AG_time += (double)(end - begin) / CLOCKS_PER_SEC;
}

void step_6() {
  //printf("Meters are decrypting the partial measurements and "
  //    "adding their self secrets...\n\n");
  int i;
  for (i = 0; i < N; i++) {
    struct meter_info *info = malloc(sizeof(struct meter_info));
    info->id = i;
    meter_add_self_secret(info);
  }
}

void step_7() {
  //printf("Power provider is summing all partial Ms to obtain the total (M)\n\n");
  clock_t begin = clock();
  int i;
  int M = 0;
  int M_real = 0;
  for (i = 0; i < N; i++) {
    M += partial_Ms[i];
  }
  clock_t end = clock();
  AG_time += (double)(end - begin) / CLOCKS_PER_SEC;
  
  for (i = 0; i < N; i++) {
    M_real += measurements[i];
  }

  //printf("Total consumption regional consumption is M = %d (real is %d)\n", M, M_real);
  //printf("%f %f\n", SM_time / N, AG_time);
  printf("%f", SM_time + AG_time);
}

int main(int argc, char *argv[]) {
  init_variables(atoi(argv[1]));
  
  steps_1_and_2();
  steps_3_and_4();
  step_5();
  step_6();
  step_7();
}
