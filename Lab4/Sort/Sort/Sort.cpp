#include <mpi.h>
#include <math.h>
#include <iostream>
#pragma comment(lib,"msmpi.lib")

using namespace std;

int main(int argc, char* argv[]){
	int n = 1000000, sum = 0, local = 0;
	int mypid, mpi_threads_num;
	double start,finish,t;

	// MPI 初始化
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_threads_num);
	start = MPI_Wtime ();

	finish = MPI_Wtime ();
	cout << "time" << finish - start << endl;
	MPI_Finalize();
	return 0;
}