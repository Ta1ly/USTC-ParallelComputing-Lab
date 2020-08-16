#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <ctime>
#pragma comment(lib,"msmpi.lib")
#define N 64


using namespace std;

void test (int* s, int size) {
	for (int i = 0; i < size; i++) {
		cout << s[i] << '\t';
	}
	cout << endl;
	return;
}

void multimerge (int* part, int *start, int size, int *merged, int n)
{
	int* index = new int[size];
	for (int i = 0; i < size; i++)
		index[i] = start[i]; // 初始化指针

	for (int i = 0; i < n; i++)
	{
		int p = 0; // 首个活跃（未达分段末尾）的指针
		while (p < size && index[p] >= start[p + 1])
			p++;
		if (p >= size)
			break;

		for (int q = p + 1; q < size; q++) // 向后遍历指针
			if (index[q] < start[q + 1])   // 指针活跃
				if (part[index[p]] > part[index[q]])
					p = q; // 取未合并分段的最小者
		merged[i] = part[index[p]++];
	}
	
	return;
}

int main (int argc, char* argv[]) {
	double start, finish, t;
	int n = N;
	int startpos, endpos;
	int* data = new int[n];
	// MPI 初始化
	int mypid, mpi_threads_num; MPI_Status status;
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &mypid);
	MPI_Comm_size (MPI_COMM_WORLD, &mpi_threads_num);
	srand ((unsigned)(time (NULL)));

	// Phase 1 初始化，Processor 0 读取数据
	if (mypid == 0) {
		for (int i = 0; i < n; i++) {
			data[i] = i + 1;
		}
		random_shuffle (data, data + n - 1);
		cout << "input"<<endl;
		test (data, n);
		start = MPI_Wtime ();
	}
	MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier (MPI_COMM_WORLD);
	
	// Phase 2 数据分配并进行局部排序，排序完成后采样
	int* sendcounts = new int[mpi_threads_num];
	int* displs = new int[mpi_threads_num + 1];
	for (int i = 0; i < mpi_threads_num; i++) {
		startpos = n * i / mpi_threads_num;
		endpos = n * (i + 1) / mpi_threads_num - 1;
		sendcounts[i] = endpos - startpos + 1;
		displs[i] = startpos;
	}
	displs[mpi_threads_num] = displs[mpi_threads_num - 1] + sendcounts[mpi_threads_num - 1];
	int localsize = sendcounts[mypid];
	int* localdata = new int[localsize];
	MPI_Scatterv (data, sendcounts, displs, MPI_INT, localdata, localsize, MPI_INT, 0, MPI_COMM_WORLD);
	sort (localdata, localdata + localsize);
	int* pivot = new int[mpi_threads_num]; 
	
	for (int i = 0; i < localsize; i++) {
		if (i % mpi_threads_num == 0) {
			pivot[i / mpi_threads_num] = localdata[i];
		}
	}
	int* pivotbuffer = new int[mpi_threads_num * mpi_threads_num];
	MPI_Gather (pivot, mpi_threads_num, MPI_INT,pivotbuffer, mpi_threads_num, MPI_INT, 0, MPI_COMM_WORLD); 
	
	// Phase 3 确认划分样本
	int* realpivot = new int[mpi_threads_num];
	if (mypid == 0) {
		//cout << "pivot num";
		//test (pivotbuffer, mpi_threads_num);
		int* head = new int[mpi_threads_num+1];
		int* mergedpivot = new int[mpi_threads_num * mpi_threads_num];
		for (int i = 0; i <= mpi_threads_num; i++) {
			head[i] = i * mpi_threads_num;
		}
		multimerge (pivotbuffer, head, mpi_threads_num, mergedpivot, mpi_threads_num * mpi_threads_num);
		for (int i = 0; i < mpi_threads_num; i++) {
			realpivot[i] = mergedpivot[(i+1) * mpi_threads_num];
		}
		//cout << "pivot" << endl;
		//test (realpivot, mpi_threads_num - 1);
	}
	MPI_Bcast (realpivot, mpi_threads_num, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier (MPI_COMM_WORLD);

	// Phase 4 根据主元样本划分
	int* part = new int[mpi_threads_num + 1];
	part[0] = 0;
	part[mpi_threads_num] = localsize;
	for (int p = 0, i = 0; p < mpi_threads_num-1; p++) {
		while (i < localsize && pivot[p] < localdata[i])  {
			i++;
		}
		part[p+1] = i;
	}
	int* partsize = new int[mpi_threads_num];
	for (int i = 0; i < mpi_threads_num; i++) {
		partsize[i] = part[i + 1] - part[i];
	}

	// Phase 5 聚集各个划分组并排序
	int* gathersize = new int[mpi_threads_num];
	for (int i = 0; i < mpi_threads_num; i++) {
		MPI_Gather (&(partsize[i]), 1, MPI_INT, gathersize, 1, MPI_INT, i, MPI_COMM_WORLD);
	}
	MPI_Barrier (MPI_COMM_WORLD);
	int gathersizesum = 0;
	for (int i = 0; i < mpi_threads_num; i++) {
		gathersizesum += gathersize[i];
	}
	int* gatherdata = new int[gathersizesum];
	displs[0] = 0;
	for (int i = 0; i < mpi_threads_num; i++) {
		displs[i + 1] = displs[i] + gathersize[i];
	}
	for (int i = 0; i < mpi_threads_num; i++) {
		MPI_Gatherv (localdata + part[i], partsize[i], MPI_INT, gatherdata, gathersize, displs, MPI_INT, i,MPI_COMM_WORLD);
	}
	int* mergedata = new int[gathersizesum];
	multimerge (gatherdata, displs, mpi_threads_num, mergedata, gathersizesum);
	//test (mergedata, gathersizesum);
	
	// Phase 6 将数据聚集到 Processor 0 中进行归并排序
	MPI_Gather (&gathersizesum, 1, MPI_INT, gathersize, 1,  MPI_INT, 0, MPI_COMM_WORLD);
	if (mypid == 0) {
		for (int i = 0; i < mpi_threads_num; i++) {
				displs[i + 1] = displs[i] + gathersize[i];
		}
	}
	MPI_Gatherv (mergedata, gathersizesum, MPI_INT, data, gathersize, displs, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Barrier (MPI_COMM_WORLD);
	
	if (mypid == 0) {
		finish = MPI_Wtime (); 
		cout << "result" << endl;
		test (data, n);
		cout << "time" << finish - start << endl;
		cout << "finish" << endl;
	}
	MPI_Finalize ();
	return 0;
}