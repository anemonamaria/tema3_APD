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

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
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
	} else if (rank == ROOT+1) {
		readFromCluster(cluster, rank);
		MPI_Send(&rank, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("M(%d,%d)\n", rank, ROOT);
		MPI_Send(&rank, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD);
		MPI_Recv(&leader, 1, MPI_INT, ROOT+2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("M(%d,%d)\n", rank, ROOT+2);
		leader = 1;
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(&rank, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
	} else if (rank == ROOT+2) {
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
	} else {
		// trimit fiecarui rank leader-ul sau
		MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	struct clusters **duplicate_cluster = (struct clusters **)malloc(sizeof(struct clusters*) * 3);
	for (int i = 0; i < 3; i ++) {
		duplicate_cluster[i] = (struct clusters *)malloc(sizeof(struct clusters));
	}

	// trimit celorlalti copii informatiile despre un vector din alt coordonator
	// si retin dimensiunea vectorilor
	if(rank == ROOT) {
		// retin dimensiunea vectorului
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Recv(&duplicate_cluster[ROOT+1]->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		MPI_Recv(&duplicate_cluster[ROOT+2]->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		// retin vectorul
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+1, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		duplicate_cluster[ROOT+1]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+1]->n);
		MPI_Recv(duplicate_cluster[ROOT+1]->neighb, duplicate_cluster[ROOT+1]->n,
			MPI_INT, ROOT+1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+2, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);

		duplicate_cluster[ROOT+2]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+2]->n);
		MPI_Recv(duplicate_cluster[ROOT+2]->neighb, duplicate_cluster[ROOT+2]->n,
			MPI_INT, ROOT+2, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	}
	// astea sunt verificate, n-ul din duplicate pt oricare root0/1/2 este retinnut bine
	if(rank == ROOT+1) {
		// retin dimensiunea vectorului
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT);
		MPI_Recv(&duplicate_cluster[ROOT]->n, 1, MPI_INT, ROOT, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		MPI_Recv(&duplicate_cluster[ROOT+2]->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);

		// retin vectorul
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT);
		duplicate_cluster[ROOT]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT]->n);
		MPI_Recv(duplicate_cluster[ROOT]->neighb, duplicate_cluster[ROOT]->n,
			MPI_INT, ROOT, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+2, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		duplicate_cluster[ROOT+2]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+2]->n);
		MPI_Recv(duplicate_cluster[ROOT+2]->neighb, duplicate_cluster[ROOT+2]->n,
			MPI_INT, ROOT+2, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	if(rank == ROOT+2) {
		// retin dimensiunea vectorului
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT);
		MPI_Recv(&duplicate_cluster[ROOT]->n, 1, MPI_INT, ROOT, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Recv(&duplicate_cluster[ROOT+1]->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		// retin vectorul
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT);
		duplicate_cluster[ROOT]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT]->n);
		MPI_Recv(duplicate_cluster[ROOT]->neighb, duplicate_cluster[ROOT]->n,
			MPI_INT, ROOT, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+1, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		duplicate_cluster[ROOT+1]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+1]->n);
		MPI_Recv(duplicate_cluster[ROOT+1]->neighb, duplicate_cluster[ROOT+1]->n,
			MPI_INT, ROOT+1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	if (rank <= 2) {
		for (int i = 0; i < 3; i++)
			qsort(duplicate_cluster[i]->neighb, duplicate_cluster[i]->n, sizeof(int), cmpfunc);
	}

	// trimit fiecarui copil dimensiunea vectorului liderului din care face parte
	if(rank == ROOT || rank == 1 || rank == 2) {
		duplicate_cluster[rank]->n = cluster->n;
		duplicate_cluster[rank]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[rank]->n);
		for(int i = 0; i < cluster->n; i++) {
			duplicate_cluster[rank]->neighb[i] = cluster->neighb[i];
		}

		for(int j = 0; j < 3; j++) {
			for(int i = 0; i < cluster->n; i++) {
				MPI_Send(&duplicate_cluster[j]->n, 1, MPI_INT, cluster->neighb[i], 2, MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, cluster->neighb[i]);
			}
		}
	} else {
		for(int j = 0; j < 3; j++) {
			MPI_Recv(&duplicate_cluster[j]->n, 1, MPI_INT, leader,
						2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			duplicate_cluster[j]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[j]->n);
		}
	}

	// trimit fiecarui copil vectorul liderului din care face parte
	if(rank == ROOT || rank == 1 || rank == 2) {
		for(int j = 0; j < 3; j++) {
			for(int i = 0; i < cluster->n; i++) {
				MPI_Send(duplicate_cluster[j]->neighb, duplicate_cluster[j]->n, MPI_INT, cluster->neighb[i], 0,
						 MPI_COMM_WORLD);
				printf("M(%d,%d)\n", rank, cluster->neighb[i]);
			}
		}
	} else {
		for(int j = 0; j < 3; j++)
			MPI_Recv(duplicate_cluster[j]->neighb, duplicate_cluster[j]->n, MPI_INT, leader,
				 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	printf("%d ->", rank);
	for (int j = 0; j < 3; j++) {
		printf(" %d:", j);
		printf("%d", duplicate_cluster[j]->neighb[0]);
		for(int i = 1; i < duplicate_cluster[j]->n; i++)
			printf(",%d", duplicate_cluster[j]->neighb[i]);
	}
	printf("\n");

	MPI_Finalize();
	return 0;
}