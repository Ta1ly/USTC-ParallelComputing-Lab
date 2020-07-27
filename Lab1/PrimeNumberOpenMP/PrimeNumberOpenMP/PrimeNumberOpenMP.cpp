#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define THREADS_NUM 4

int isPrime (int num) {
	int i;
	for (i = 2; i <= sqrt ((double)num); i++) {
		if (num % i == 0) return 0;
	}
	return 1;
}

int main (){
	clock_t start, finish, t1, t2;
	int sum = 0;
	int sumi[THREADS_NUM] = {};
	int n = 1000000;
	omp_set_num_threads (THREADS_NUM);
	start = clock ();
#pragma omp parallel shared(sum,sumi)
	{
		int i;
		int t = omp_get_thread_num ();
		for (i = 1 + t * 2; i <= n; i += THREADS_NUM* 2) {
			 sumi[t] += isPrime(i);
		}
	}
	for (int i = 0; i < THREADS_NUM; i++) {
		sum += sumi[i];
	}
	finish = clock ();
	t1 = finish - start;
	printf ("parallel computing in %d threads:\n",THREADS_NUM);
	printf ("Prime number num:%d\n",sum);
	printf ("time: %ldms\n\n",t1);
	sum = 0;
	start = clock ();
	for (int i = 1; i <= n; i += 2) {
		sum += isPrime (i);
	}
	finish = clock ();
	t2 = finish - start;
	printf ("serial computing:\n");
	printf ("Prime number num:%d\n", sum);
	printf ("time: %ldms\n\n", t2);
	printf ("Speedup: %lf", (double)t2 / (double)t1);
	return 0;
}