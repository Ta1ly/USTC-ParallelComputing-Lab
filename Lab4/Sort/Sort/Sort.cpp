#include <mpi.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#pragma comment(lib,"msmpi.lib")

using namespace std;

int * read_data (FILE * fp, int & n) {
	int i = 0;
	fscanf (fp, "%d", &n);
	int * data = (int*)malloc (sizeof (int) * n);
	while (i < n) {
		fscanf (fp, "%d", &(data[i]));
		i++;
	}
	return data;
}

int main(int argc, char* argv[]){
	
	double start,finish,t;
	int n = 1;
	int startpos, endpos;
	int *data = NULL;
	// MPI 初始化
	int mypid, mpi_threads_num; MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_threads_num);
	
    // Phase 1
	if (mypid == 0) {
		FILE* fp = fopen ("input.txt", "r");
		data = read_data (fp,n);
		/*for (int i = 0; i < n; i++) {
			cout << data[i] << endl;
		}*/
		fclose (fp);
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		start = MPI_Wtime ();
	}
	MPI_Barrier(MPI_COMM_WORLD);
	// Phase 2
	if (mypid == 0) {
		for (int i = 0; i < mpi_threads_num; i++){
			if (i != 0) {
				startpos = n * i / mpi_threads_num;
				endpos = n * (i + 1) / mpi_threads_num - 1;
				MPI_Send(&(data[startpos]), endpos - startpos + 1, MPI_INT, i, i, MPI_COMM_WORLD);
			}
		}
		startpos = n * mypid / mpi_threads_num;
		endpos = n * (mypid + 1) / mpi_threads_num - 1;
	}
	else {
		startpos = n * mypid / mpi_threads_num;
		endpos = n * (mypid + 1) / mpi_threads_num - 1;
		data = (int *)malloc(sizeof(int) * (endpos - startpos + 1));
		memset (data, 0, sizeof (int) * (endpos - startpos + 1));
		MPI_Recv(data, endpos - startpos + 1, MPI_INT, 0, mypid, MPI_COMM_WORLD, &status);
	}
	sort (data, data + endpos - startpos+1);
	for (int i = startpos; i <= endpos; i++) {
		cout << data[i] << endl;
	}
	//
	if (mypid == 0){
		finish = MPI_Wtime ();
		cout << "time" << finish - start << endl;
	}
	
	MPI_Finalize();
	return 0;
}