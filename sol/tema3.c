// Moraru Anemona Maria 333CC

#include<mpi.h>
#include<stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct clusters {
	int *neighb;
	int n;
} clusters;

struct clusters *cluster;  // todo make this static not global


void readFromCluster(FILE *file) {
	cluster = malloc(sizeof(clusters) * 1);

	fscanf(file, "%d", &cluster->n);
	cluster->neighb = malloc(sizeof(int) * cluster->n);

	for(int i = 0; i < cluster->n; i++) {
		fscanf(file, "%d", &cluster->neighb[i]);
	}
}

int main(int argc, char * argv[]) {
	int rank, nProcesses, num_procs, leader;
	int *parents, **topology;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Request request;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);


	if (rank == 0) {
		readFromCluster(fopen("cluster0.txt", "r"));
	}
	if (rank == 1) {
		readFromCluster(fopen("cluster1.txt", "r"));
	}
	if (rank == 2) {
		readFromCluster(fopen("cluster2.txt", "r"));
	}

	if(rank == 0) {
		printf("%d\n", cluster->n);
	}
	MPI_Finalize();
	return 0;
}