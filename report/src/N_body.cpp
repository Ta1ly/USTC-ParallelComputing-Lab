#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#pragma comment(lib,"msmpi.lib")
#define M 10000
#define N 256
#define MIN_DISTANCE 0.0001
#define G (6.67 * pow(10.0,-11.0))
#define SLICE 0.0001

using namespace std;

typedef struct body{
	double x;
	double y;
	double f_x;
	double f_y;
	double v_x;
	double v_y;
}Body;

void force(Body* balls, int num) {
	int i;
	double f_x = 0, f_y = 0,dx,dy;
	for (i = 0; i < N; i++) {
		if (i != num) {
			dx = balls[i].x - balls[num].x;
			dy = balls[i].y - balls[num].y;
			if (abs (dx) < MIN_DISTANCE) {
				if (dx > 0)dx = MIN_DISTANCE;
				else dx = -MIN_DISTANCE;
			}
			if (abs (dy) < MIN_DISTANCE) {
				if (dy > 0)dy = MIN_DISTANCE;
				else dy = -MIN_DISTANCE;
			}
			f_x += G * M * M * dx / pow (dx * dx + dy * dy, 1.5);
			f_y += G * M * M * dy / pow (dx * dx + dy * dy, 1.5);
		}
	}
	balls[num].f_x = f_x;
	balls[num].f_y = f_y;
	return;
}

void velocities(Body* balls, int num) {
	balls[num].v_x += balls[num].f_x / M * SLICE;
	balls[num].v_y += balls[num].f_y / M * SLICE;
	return;
}

void positions(Body* balls, int num) {
	balls[num].x += balls[num].v_x * SLICE;
	balls[num].y += balls[num].v_y * SLICE;
}

int main(int argc, char* argv[]){
	int iternum = 100;
	int i,j;
	int mypid, mpi_threads_num;
	MPI_Status status;
	double start, finish, t;
	Body* balls = (Body*)malloc (sizeof (Body) * 256);
	int n = sqrt (N), startpos,endpos;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			balls[i * n + j].x = i*0.01;
			balls[i * n + j].y = j * 0.01;
			balls[i * n + j].f_x = 0;
			balls[i * n + j].f_y = 0;
			balls[i * n + j].v_x = 0;
			balls[i * n + j].v_y = 0;
		}
	}
	
	// MPI 初始化
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_threads_num);
	start = MPI_Wtime ();
	startpos = N * mypid / mpi_threads_num;
	endpos = (double)N * (mypid+1) / mpi_threads_num - 1.0;
	//cout << startpos << endpos << endl;
	while (iternum--) {
		for (i = startpos; i <= endpos; i ++) {
			force (balls, i);
		}
		for (i = startpos; i <= endpos; i++) {
			velocities (balls, i);
			positions (balls, i);
		}
		MPI_Barrier (MPI_COMM_WORLD);
		if (mypid != 0) {
			// 将更新的位置信息发送给 thread 0
			MPI_Send (&(balls[startpos]), 6 * (endpos - startpos + 1), MPI_INT, 0, iternum * 10 + mypid, MPI_COMM_WORLD);
		}
		else {
			for (i = 0; i < mpi_threads_num; i++) {
				// thread 0 接收来自其他 thread 的位置信息
				if (i != 0) {
					MPI_Recv (&(balls[N * i / mpi_threads_num]), 6 * N, MPI_INT, i, iternum * 10 + i, MPI_COMM_WORLD, &status);
				}
			}
			// 接收到全部信息后广播，各个thread 更新位置信息
			MPI_Bcast (balls, 6 * N, MPI_INT, 0, MPI_COMM_WORLD);
		}
		MPI_Barrier (MPI_COMM_WORLD);
	}
	finish = MPI_Wtime ();
	if (mypid == 0) {
		/*
		for (i = 0; i < N; i++) {
			cout << setw (8) << i << "\t";
			cout << setw (8) << balls[i].x << "\t" << setw (8) << balls[i].y << "\t";
			cout << setw (8) << balls[i].f_x << "\t" << setw (8) << balls[i].f_y << "\t";
			cout << setw (8) << balls[i].v_x << "\t" << setw (8) << balls[i].v_y << endl;
		}*/
		cout << "threads num:" << mpi_threads_num << endl;
		cout << "time:" << finish - start << endl;
	}

	MPI_Finalize();
	return 0;
}
