#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#pragma comment(lib,"msmpi.lib")
#define V_MAX 70
#define P 0.2

using namespace std;

int move (int v, int d) {
	if (v < d) {
		if (v < V_MAX) v++;
	}
	else {
		v = d;
	}
	double r = (rand () % 100) / 100.0;
	if (r < P && v > 0) {
		v--;
	}
	return v;
}

int main(int argc, char* argv[]){
	srand ((int)time (0));
	int car_num = 100;
	int* car_speed = (int*)malloc (sizeof (int) * car_num);
	int* car_pos = (int*)malloc (sizeof (int) * car_num);
	for (int i = 0; i < car_num; i++) {
		car_pos[i] = i;
		car_speed[i] = 0;
	}

	int mypid, mpi_threads_num;
	double start, finish;
	// MPI 初始化
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_threads_num);

	// 信息传递
	MPI_Bcast(&car_pos, 1, MPI_INT, 0, MPI_COMM_WORLD);
	start = MPI_Wtime();

	if (mypid == 0) {

	}




	// 归约，将各个local中数据集中到sum中
	MPI_Reduce(&local, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	finish= MPI_Wtime();
	t = finish - start;




	MPI_Finalize();
	return 0;
}