#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <gmp.h>

int number_of_meters;
int *individual_measurements;

mpz_t **enc_measurements;

mpz_t *aggregated_c;
mpz_t *aggregated_d;
mpz_t *D;

// Each meter starts with a large prime number, and a G generator
mpz_t *prime_q;
mpz_t *generator;

// Each meter starts with a public and a secret, partial keys
struct el_gamal_pub_key{
	mpz_t yi;
};

struct el_gamal_priv_key{
	mpz_t xi;
};

// The aggregated yis compose the global public key
struct el_gamal_pub_global_key{
	mpz_t y;
};

mpz_t *set_of_Tis;
mpz_t *random_zis;
struct el_gamal_pub_key *pubkeys;
struct el_gamal_priv_key *secretkeys;

// Variables needed to compute the time to perform the operations
double SM_time = 0;
double AG_time = 0;

struct meter_info{
	int id;
};

struct el_gamal_pub_global_key *global_public_key;
struct el_gamal_pub_key *public_key;
struct el_gamal_priv_key *secret_key;

// Variables needed to generate random numbers

gmp_randstate_t rand_st;

time_t timestamp;
double log_time;
double log_time_ml;

void init_variables(int n){
	srand(time(NULL));
	timestamp = time(NULL);
	gmp_randinit_mt(rand_st);
	gmp_randseed_ui(rand_st, timestamp);

	number_of_meters = n;

	prime_q = malloc(sizeof(mpz_t));
	mpz_init(*prime_q);

	individual_measurements = calloc(number_of_meters, sizeof(int));
	enc_measurements = calloc(number_of_meters, sizeof(mpz_t));	
	random_zis = calloc(number_of_meters, sizeof(mpz_t));
	set_of_Tis = calloc(number_of_meters, sizeof(mpz_t));

	generator = malloc(sizeof(mpz_t));	
	mpz_init(*generator);

	pubkeys = calloc(number_of_meters, sizeof(struct el_gamal_pub_key));
	secretkeys = calloc(number_of_meters, sizeof(struct el_gamal_priv_key));

	global_public_key = malloc(sizeof(struct el_gamal_pub_global_key));
	public_key = malloc(sizeof(struct el_gamal_pub_key));
	secret_key = malloc(sizeof(struct el_gamal_priv_key));

	int i;
	for(i = 0; i < number_of_meters; i++){
		enc_measurements[i] = calloc(2, sizeof(mpz_t)); //enc = (ci, di)
		individual_measurements[i] = (rand() % 2016) + 201;
	}
}

mpz_t* find_prime(){
	
	
	mpz_t *q = malloc(sizeof(mpz_t));
	mpz_init(*q);
	
	do{
		mpz_urandomb(*q, rand_st, 2048 / 2);
	}while(!mpz_probab_prime_p(*q, 10));

	return q;

}

mpz_t* find_primitive_root(mpz_t *p){
	mpz_t *result = malloc(sizeof(mpz_t));
	mpz_init(*result);

	// p - 1 will be necessary throughout the computation of the root	
	mpz_t p_minus1;
	mpz_init(p_minus1);
	mpz_sub_ui(p_minus1, *p, 1);
	
	if(mpz_cmp_ui(*p, 2) == 0){
		mpz_set_ui(*result, 1);
		return result;
	}else{
		mpz_t p1;
		mpz_init(p1);
		mpz_set_ui(p1, 2);

		mpz_t p2;
		mpz_init(p2);

		mpz_fdiv_q(p2, p_minus1, p1);

		while(1){
			mpz_urandomm(*result, rand_st, *p);

			if(!(mpz_cmp_ui(*result, 0) == 0 || mpz_cmp_ui(*result, 1) == 0)){

				mpz_t temp;
				mpz_init(temp);

				mpz_powm(temp, *result, p2, *p);

				if(!(mpz_cmp_ui(temp, 1) == 0)){

					mpz_t exponent;
					mpz_init(exponent);

					mpz_fdiv_q(exponent, p_minus1, p2);
					
					mpz_powm(temp, *result, exponent, *p);
					if(!(mpz_cmp_ui(temp, 1) == 0)){
						return result;
					}
				}

			}
		}
	}
}

void generate_keys(){
	// Partial secret and public keys
	mpz_init(secret_key->xi);

	mpz_t limit; mpz_init(limit);
	mpz_sub_ui(limit, *prime_q, 2);

	int i;
	for(i = 0; i < number_of_meters; i++){
		mpz_init(secretkeys[i].xi);
		mpz_urandomm(secretkeys[i].xi, rand_st, limit);
		mpz_add_ui(secretkeys[i].xi, secretkeys[i].xi, 1);

		mpz_init(pubkeys[i].yi);

		mpz_powm(pubkeys[i].yi, *generator, secretkeys[i].xi, *prime_q);
    
	
	}

	// Global public key
	mpz_init(global_public_key->y);
	mpz_set_ui(global_public_key->y, 1);

	for(i = 0; i < number_of_meters; i++){
		mpz_mul(global_public_key->y, global_public_key->y, pubkeys[i].yi);
	}
	mpz_mod(global_public_key->y, global_public_key->y, *prime_q);

}

void meter_encrypt_send(struct meter_info *info){

	/*
	  	First, we generate random numbers (z), for the meter will encrypt the value:
		generator ^ (mi + zi)
	*/
	mpz_t z; mpz_init(z);
	mpz_urandomm(z, rand_st, *prime_q);
	
	mpz_init(random_zis[info->id]);
	
	mpz_set(random_zis[info->id], z);
	
	mpz_t encrypt_value; mpz_init(encrypt_value);
	
	mpz_t zm; mpz_init(zm); //zm is zi + mi
	mpz_add_ui(zm, z, individual_measurements[info->id]);

  	mpz_powm(encrypt_value, *generator, zm, *prime_q);

	// Encrypting measurement
	mpz_t r; mpz_init(r);
	mpz_urandomm(r, rand_st, *prime_q);

	mpz_t c; mpz_init(c);
	mpz_powm(c, *generator, r, *prime_q);

	mpz_t d; mpz_init(d);
	mpz_powm(d, global_public_key->y, r, *prime_q);
	mpz_mul(d, encrypt_value, d);
  mpz_mod(d, d, *prime_q);
	// Sending encrypted measurement to aggregator
	mpz_init(enc_measurements[info->id][0]); mpz_init(enc_measurements[info->id][1]);
	
	mpz_set(enc_measurements[info->id][0], c);
	mpz_set(enc_measurements[info->id][1], d);
}

void meter_calculate_Ti(struct meter_info *info){

	mpz_t Ti; mpz_init(Ti);
	mpz_t temp_operand; mpz_init(temp_operand);

	mpz_powm(Ti, *aggregated_c, secretkeys[info->id].xi, *prime_q);
	mpz_powm(temp_operand, *generator, random_zis[info->id], *prime_q);
	mpz_mul(Ti, Ti, temp_operand);
  mpz_mod(Ti, Ti, *prime_q);

	mpz_init(set_of_Tis[info->id]);	mpz_set(set_of_Tis[info->id], Ti);
}

int try_log(mpz_t result, mpz_t base){

	mpz_t partial; mpz_init(partial);
	mpz_t exponent; mpz_init(exponent);

	int max = 2217 * number_of_meters;

	int i;
	for(i = (201 * number_of_meters); i <= max; i++){
	
		mpz_set_ui(exponent, i);
		mpz_powm(partial, base, exponent, *prime_q);

		if(mpz_cmp(partial, result) == 0){
			return mpz_get_ui(exponent);
		}
	
	}

	return -1;

}

/*
	In the step 1, we will generate all the necessary information within the meters
*/
void step_1(){

	mpz_set(*prime_q, *(find_prime()));
	mpz_set(*generator, *(find_primitive_root(prime_q)));

	generate_keys();

}

/*
	Here, meters encrypt their measurements and send to the aggregator
*/
void step_3(){

	clock_t SM_begin = clock();

	int i;
	for(i = 0; i < number_of_meters; i++){
		struct meter_info *info = malloc(sizeof(struct meter_info));
		info->id = i;
		meter_encrypt_send(info);
		free(info);
	}

	clock_t SM_end = clock();
	SM_time += (double)(SM_end - SM_begin)/CLOCKS_PER_SEC;

}

/*
	Aggregator aggregates the measurements
*/
void step_4(){

	clock_t AG_begin = clock();

	aggregated_c = malloc(sizeof(mpz_t));
	aggregated_d = malloc(sizeof(mpz_t));
	mpz_init(*aggregated_c); mpz_init(*aggregated_d);
	mpz_set_ui(*aggregated_c, 1);

	mpz_set_ui(*aggregated_d, 1);

	int i;
	for(i = 0; i < number_of_meters; i++){
		mpz_mul(*aggregated_c, *aggregated_c, enc_measurements[i][0]);
		mpz_mul(*aggregated_d, *aggregated_d, enc_measurements[i][1]);
	}
  mpz_mod(*aggregated_c, *aggregated_c, *prime_q);
  mpz_mod(*aggregated_d, *aggregated_d, *prime_q);

	clock_t AG_end = clock();
	AG_time += (double)(AG_end - AG_begin)/CLOCKS_PER_SEC;

}

/*
	Each meter now calculates a T value
*/
void step_5(){

	clock_t SM_begin = clock();

	int i;
	for(i = 0; i < number_of_meters; i++){
		struct meter_info *info = malloc(sizeof(struct meter_info));
		info->id = i;

		meter_calculate_Ti(info);

		free(info);
	}	
	
	clock_t SM_end = clock();
	SM_time += (double)(SM_end - SM_begin)/CLOCKS_PER_SEC;

}

/*
	The Aggregator now aggregates all Tis
*/
void step_6(){

	clock_t AG_begin = clock();
	
	D = malloc(sizeof(mpz_t));
	mpz_init(*D);
	mpz_set_ui(*D, 1);

	int i;
	for(i = 0; i < number_of_meters; i++){
		mpz_mul(*D, *D, set_of_Tis[i]);
	}
	mpz_mod(*D, *D, *prime_q);
  
	mpz_invert(*D, *D, *prime_q);
	mpz_mul(*D, *aggregated_d, *D);
	mpz_mod(*D, *D, *prime_q);

	int log = try_log(*D, *generator);
	
	clock_t AG_end = clock();
	AG_time += (double)(AG_end - AG_begin)/CLOCKS_PER_SEC;

//	if(log == -1){
//		puts("The log function failed");
//	}else{
//		printf("Busom consumption: %d\n", log);
//	}

//	gmp_printf("D: %Zd\n", D);

	// Computing the actual total consumption
	int sum = 0;
	for(i = 0; i < number_of_meters; i++){
		sum += (individual_measurements[i]);
	}

	mpz_t result; mpz_init(result); mpz_set_ui(result, 1);
	mpz_t partial_c; mpz_init(partial_c);

	for(i = 0; i < number_of_meters; i++){
		mpz_set_ui(partial_c, individual_measurements[i]);
		mpz_powm(partial_c, *generator, partial_c, *prime_q);
		mpz_mul(result, result, partial_c);
	}
  mpz_mod(result, result, *prime_q);

//	gmp_printf("S: %Zd\n", result);

//  gmp_printf("g: %Zd\n", *generator);
//  gmp_printf("q: %Zd\n", *prime_q);
  
//	printf("Real consumption: %d\n", sum);

//	printf("Log time: %f\n", log_time);
	printf("%f %f\n", SM_time/number_of_meters, AG_time);
}

int main(int argc, char *argv[]){
	init_variables(atoi(argv[1]));
	step_1();
	// Step 2 is not necessary, for it deals with meters setup
	step_3();
	step_4();
	step_5();
	step_6();
}
