// Moraru Anemona Maria 333CC

#include<mpi.h>
#include<stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define CONVERGENCE_COEF 100
#define ROOT 0

typedef struct clusters {
	int *neighb;
	int n;
} clusters;


void readFromCluster(struct clusters *cluster, int rank) {
	FILE *fp;
    char file_name[15];
    sprintf(file_name, "./cluster%d.txt", rank);

    fp = fopen(file_name, "r");
	fscanf(fp, "%d", &cluster->n);

	cluster->neighb = malloc(sizeof(int) * cluster->n);
	for(int i = 0; i < cluster->n; i++) {
		fscanf(fp, "%d", &cluster->neighb[i]);
	}

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

	// citit, mesaje, topo
	if (rank == ROOT) {
		readFromCluster(cluster, rank);
		MPI_Send(&rank, 1, MPI_INT, ROOT+1, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT+1, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Send(&rank, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT+2);
		leader = 0;
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(&rank, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
	}
	if (rank == ROOT+1) {
		readFromCluster(cluster, rank);
		MPI_Send(&rank, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT);
		MPI_Send(&rank, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT+2);
		leader = 1;
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(&rank, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
	}
	if (rank == ROOT+2) {
		readFromCluster(cluster, rank);
		MPI_Send(&rank, 1, MPI_INT, ROOT+1, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT+1, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Send(&rank, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &status);
		printf("M(%d,%d)\n", rank, ROOT);
		leader = 2;
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(&rank, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
	}


	// trimit fiecarui rank leader-ul sau
	if (rank != ROOT && rank != 1 && rank != 2) {
		MPI_Status status;
		MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	}

	// if(rank == 0) {
	// 	for(int i = 0; i < cluster->n; i ++) {
	// 		printf("%d ", cluster->neighb[i]);
	// 	}
	// 	printf("\n");
	// }   // ---> e bine

		//printf("rank %d cluster %d leader\n", rank, leader);

	// pentru topo
	/*
		prima data realizam comunicarea intre coordonatori
		apoi realizam comunicarea intre copii si comunicatori si invers
		in tot acest timp am afisat la termina M(sursa, destinatia)
		si apoi afisam topologia
	*/
	MPI_Finalize();
	return 0;
}