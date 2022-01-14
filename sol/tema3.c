// Moraru Anemona Maria 333CC

#include<mpi.h>
#include<stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define ROOT 0

typedef struct clusters {
	int *neighb;
	int n;
} clusters;


void readFromCluster(struct clusters *cluster, FILE *file) {
	fscanf(file, "%d", &cluster->n);
	cluster->neighb = malloc(sizeof(int) * cluster->n);

	for(int i = 0; i < cluster->n; i++) {
		fscanf(file, "%d", &cluster->neighb[i]);
	}
}

int iAmYourCoordinator(struct clusters *cluster, int rank) {
	int coordinator = -1;
	coordinator = rank;
	int new_coordinator;
	for(int i = 0; i < cluster->n; i++) {
		MPI_Send(&coordinator, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
		MPI_Recv(&new_coordinator, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	return new_coordinator;
}

int main(int argc, char * argv[]) {
	int rank, nProcesses, num_procs, leader;
	int *parents, **topology;

	struct clusters *cluster = (struct clusters *)malloc(sizeof(struct clusters));
	int coordinator;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Request request;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);


	if (rank == ROOT) {
		readFromCluster(cluster, fopen("cluster0.txt", "r"));
		coordinator = iAmYourCoordinator(cluster, rank);
	}
	else if (rank == 1) {
		readFromCluster(cluster, fopen("cluster1.txt", "r"));
		coordinator = iAmYourCoordinator(cluster, rank);
	}
	else if (rank == 2) {
		readFromCluster(cluster, fopen("cluster2.txt", "r"));
		coordinator = iAmYourCoordinator(cluster, rank);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(rank == ROOT) {
		printf("%d\n", cluster->n);
	}
	MPI_Finalize();
	return 0;
}