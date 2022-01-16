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

	// printf("%d ->", rank);
	struct clusters **duplicate_cluster = (struct clusters **)malloc(sizeof(struct clusters*) * 3);
	for (int i = 0; i < 3; i ++) {
		duplicate_cluster[i] = (struct clusters *)malloc(sizeof(struct clusters));
	}


	// trimit fiecarui copil dimensiunea vectorului liderului din care face parte
	int size_sent;
	int *v_recv;
	if(rank == ROOT || rank == 1 || rank == 2) {
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(&cluster->n, 1, MPI_INT, cluster->neighb[i], 2, MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
		duplicate_cluster[rank]->n = cluster->n;
		duplicate_cluster[rank]->neighb = (int *)calloc(sizeof(int), duplicate_cluster[rank]->n);
	}

	if (rank != ROOT && rank != 1 && rank != 2) {
		MPI_Status status;
		MPI_Recv(&duplicate_cluster[leader]->n, 1, MPI_INT, MPI_ANY_SOURCE, 2,
					MPI_COMM_WORLD, &status);
	}

	// trimit fiecarui copil vectorul liderului din care face parte
	if(rank == ROOT || rank == 1 || rank == 2) {
		for(int i = 0; i < cluster->n; i++) {
			MPI_Send(cluster->neighb, cluster->n, MPI_INT, cluster->neighb[i], 0,
					 MPI_COMM_WORLD);
			printf("M(%d,%d)\n", rank, cluster->neighb[i]);
		}
		duplicate_cluster[rank]->neighb = cluster->neighb;
	}

	if (rank != ROOT && rank != 1 && rank != 2) {
		MPI_Status status;
		MPI_Recv(duplicate_cluster[leader]->neighb, duplicate_cluster[leader]->n, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	}

	// trimit celorlalti copii informatiile despre un vector din alt coordonator
	// si retin dimensiunea vectorilor
	if(rank == ROOT) {
		// TODO de ce se schimba dimesniunea lui duplicate_cluster[ROOT+2]->n ?!
		// retin dimensiunea vectorului
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Recv(&duplicate_cluster[ROOT+1]->n, 1, MPI_INT, ROOT+1, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
		MPI_Send(&cluster->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		MPI_Recv(&duplicate_cluster[ROOT+2]->n, 1, MPI_INT, ROOT+2, 2, MPI_COMM_WORLD,
			MPI_STATUS_IGNORE);
			printf("asta e n %d din root\n", duplicate_cluster[ROOT+2]->n);
		// retin vectorul
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+1, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		MPI_Recv(duplicate_cluster[ROOT+1]->neighb, duplicate_cluster[ROOT+1]->n,
			MPI_INT, ROOT+1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// for(int i = 0; i < duplicate_cluster[ROOT+1]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT+1]->neighb[i]);
		// }  ---> asta pt root+1 este retinuta bine
		//printf("\n");
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+2, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		duplicate_cluster[ROOT+2]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+2]->n);
		MPI_Recv(duplicate_cluster[ROOT+2]->neighb, duplicate_cluster[ROOT+2]->n,
			MPI_INT, ROOT+2, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// TODO de ce retine alta valoare?
		printf("asta e n %d tot din root dar mai jos\n", duplicate_cluster[ROOT+2]->n);
		// TODO  de ce nu sunt retinuti ok vectorii astia?!
		// for(int i = 0; i < duplicate_cluster[ROOT+2]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT+2]->neighb[i]);
		// }
		// printf("\n");
		// for(int i = 0; i < duplicate_cluster[ROOT]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT]->neighb[i]);
		// }
		// printf("\n");
		// for(int i = 0; i < duplicate_cluster[ROOT+1]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT+1]->neighb[i]);
		// }
		// printf("\n");  ---> aici nu mai este retinuta ok
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
			printf("asta e n %d din root+1\n", duplicate_cluster[ROOT+2]->n);

		// retin vectorul
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT);
		MPI_Recv(duplicate_cluster[ROOT]->neighb, duplicate_cluster[ROOT]->n,
			MPI_INT, ROOT, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+2, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+2);
		MPI_Recv(duplicate_cluster[ROOT+2]->neighb, duplicate_cluster[ROOT+2]->n,
			MPI_INT, ROOT+2, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// for(int i = 0; i < duplicate_cluster[ROOT+2]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT+2]->neighb[i]);
		// }
		// printf("\n");
		// for(int i = 0; i < duplicate_cluster[ROOT]->n; i++) {
		// 	printf("%d ", duplicate_cluster[ROOT]->neighb[i]);
		// }
		// printf("\n");
		// for(int i = 0; i < duplicate_cluster[ROOT+1]->n; i++) {  // TODO asta da eroare .. dc?!
		// 	printf("%d ", duplicate_cluster[ROOT+1]->neighb[i]);
		// }
		// printf("\n");
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
		MPI_Recv(duplicate_cluster[ROOT]->neighb, duplicate_cluster[ROOT]->n,
			MPI_INT, ROOT, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(cluster->neighb, cluster->n, MPI_INT, ROOT+1, 3, MPI_COMM_WORLD);
		printf("M(%d,%d)\n", rank, ROOT+1);
		duplicate_cluster[ROOT+1]->neighb = (int *)malloc(sizeof(int) * duplicate_cluster[ROOT+1]->n);
		MPI_Recv(duplicate_cluster[ROOT+1]->neighb, duplicate_cluster[ROOT+1]->n,
			MPI_INT, ROOT+1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for(int i = 0; i < duplicate_cluster[ROOT+2]->n; i++) {
			printf("%d ", duplicate_cluster[ROOT+2]->neighb[i]);
		}
		printf("\n");
		for(int i = 0; i < duplicate_cluster[ROOT]->n; i++) {  // TODO asta de ce e retinuta prost!?
			printf("%d ", duplicate_cluster[ROOT]->neighb[i]);
		}
		printf("\n");
		for(int i = 0; i < duplicate_cluster[ROOT+1]->n; i++) {  // TODO asta da eroare .. dc?!
			printf("%d ", duplicate_cluster[ROOT+1]->neighb[i]);
		}
		printf("\n");
	}

	// if(rank == 1) {
	// 	for(int i = 0; i < duplicate_cluster[0]->n; i++) {
	// 		printf("%d ", duplicate_cluster[0]->neighb[i]);
	// 	}
	// 	printf("\n");
	// 	for(int i = 0; i < duplicate_cluster[1]->n; i++) {
	// 		printf("%d ", duplicate_cluster[1]->neighb[i]); // nu retii ce trebuie?!
	// 	}
	// 	printf("\n");
	// 	for(int i = 0; i < duplicate_cluster[2]->n; i++) {
	// 		printf("%d ", duplicate_cluster[2]->neighb[i]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");

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