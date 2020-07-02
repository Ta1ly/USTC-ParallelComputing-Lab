#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include "string.h"
#pragma comment(lib,"msmpi.lib")

int main(int argc, char* argv[]){
	long iternum = 10000000;
	double sum = 0, local = 0;
	int mypid, mpi_threads_num;
	double starttime1, endtime1, starttime2, endtime2, t1, t2;

	// MPI 初始化
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_threads_num);

	// 信息传递
	MPI_Bcast(&iternum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	starttime1 = MPI_Wtime();

	// 计算质数数量并存储在本地local
	for (int i = mypid + 1; i <= iternum; i += mpi_threads_num) {
		local += 1.0 / i / i;
	}

	// 归约，将各个local中数据集中到sum中
	MPI_Reduce(&local, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	endtime1 = MPI_Wtime();
	t1 = endtime1 - starttime1;

	if (mypid == 0){
		printf ("Parallel Computing in %d Thread(s)\n", mpi_threads_num);
		printf("time:	% f\n", t1);
		printf("Pi result： %lf\n\n", sqrt(sum*6));
	}

	// 计算单线程时间和加速比
	sum = 0;
	starttime2 = MPI_Wtime();
	if (mypid == 0){
		for (int i = 1; i <= iternum; i ++) {
			sum += 1.0 / i / i;
		}
		endtime2 = MPI_Wtime();
		t2 = endtime2 - starttime2;
		printf ("Serial Computing in Thread %d\n", mypid);
		printf ("time:	% f\n", t2);
		printf ("Pi result： %lf\n\n", sqrt (sum * 6));

		printf ("Speedup: %f", t2 / t1);
	}
	MPI_Finalize();
	return 0;
}