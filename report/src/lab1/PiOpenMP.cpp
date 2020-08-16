#include <iostream>
#include <math.h>
#include <omp.h>
#include <windows.h>
#define THREADS_NUM 8
using namespace std;

int main () {
	long iternum = 500000;
	double sum = 0;
	double sumi[THREADS_NUM] = {};
	double t1, t2;
	LARGE_INTEGER start, finish, cc;
	QueryPerformanceFrequency (&cc);
	omp_set_num_threads (THREADS_NUM);

	QueryPerformanceCounter (&start);
	#pragma omp parallel shared(sum,sumi) 
	{
		int t = omp_get_thread_num ();
		
		for (int i = 1 + t ; i < iternum; i += THREADS_NUM) {
			sumi[t] += 1.0  /  i  / i; 
		}
		/*
		for (int i = t; i < iternum; i += THREADS_NUM) {
			int sig = i % 2 == 0 ? 1 : -1;
			sumi[t] += (double)sig / (i + i + 1);
		}*/
	}
	
	for(int i = 0; i < THREADS_NUM; i++) {
		sum += sumi[i];
	}
	QueryPerformanceCounter (&finish);
	t1 = ((double)finish.QuadPart - (double)start.QuadPart) / (double)cc.QuadPart;

	cout << "Parallel Computing with " << THREADS_NUM << " Threads" << endl;
	cout<<"Pi is " << sqrt(sum*6) << endl;
	cout << "time: " << t1 << "s" << endl << endl;
	sum = 0; QueryPerformanceCounter (&start);
	for (int i = 1; i <= iternum; i++) {
		sum += 1.0 / i / i;
	}
	QueryPerformanceCounter (&finish);
	t2 = ((double)finish.QuadPart - (double)start.QuadPart) / (double)cc.QuadPart;
	cout << "Serial Computing"<< endl;
	cout << "Pi is " << sqrt(sum*6) << endl;
	cout << "time: " << t2 << "s" << endl << endl;
	cout << "Speedup = " << (double)t2 / (double)t1 << endl;

	return 0;
}