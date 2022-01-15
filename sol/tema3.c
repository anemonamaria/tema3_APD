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
	// printf("din functie\n");
	cluster->neighb = malloc(sizeof(int) * cluster->n);
	for(int i = 0; i < cluster->n; i++) {
		fscanf(fp, "%d", &cluster->neighb[i]);
	}
}

int iAmYourCoordinator(struct clusters *cluster, int rank) {
	int coordinator = -1;
	coordinator = rank;
	// for (int k = 0; k < CONVERGENCE_COEF; k++) {
		for(int i = 0; i < cluster->n; ++i) {
			MPI_Send(&coordinator, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			int new_coordinator;
			MPI_Recv(&new_coordinator, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}  // ?????? what at rcv?
	// }
	printf("intri?\n");

	return coordinator;
}

int leader_chosing(struct clusters *cluster,int rank, int nProcesses) {
	int leader = -1;
	int q;
	leader = rank;

	for (int k = 0; k < CONVERGENCE_COEF; k++) {
		for (int i = 0; i < cluster->n; ++i) {
			MPI_Send(&leader, 1, MPI_INT, cluster->neighb[i], 0, MPI_COMM_WORLD);
			int new_leader;
			MPI_Recv(&new_leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("intri?\n");  //dc???     // MPI_ANY_SOURCE sau rank?

			if (new_leader > leader)
				leader = new_leader;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	printf("%i/%i: leader is %i\n", rank, nProcesses, leader);

	return leader;
}

// todo de anuntat procesele copil cine este coordonatorul
// todo de facut o functie care afiseaza topologia pentru procese
// in momentul in care acesta o afla

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
		readFromCluster(cluster, rank);
		// printf("\n%d\n", cluster->n);
		//coordinator = iAmYourCoordinator(cluster, rank);
		// MPI_Barrier(MPI_COMM_WORLD);
		leader = leader_chosing(cluster, rank, nProcesses);

	}
	else if (rank == 1) {
		readFromCluster(cluster, rank);
		// printf("\n%d\n", cluster->n);
		// coordinator = iAmYourCoordinator(cluster, rank);
		// MPI_Barrier(MPI_COMM_WORLD);
		leader = leader_chosing(cluster, rank, nProcesses);
	}
	else if (rank == 2) {
		readFromCluster(cluster, rank);
		// printf("\n%d\n", cluster->n);
		// coordinator = iAmYourCoordinator(cluster, rank);
		// MPI_Barrier(MPI_COMM_WORLD);
		leader = leader_chosing(cluster, rank, nProcesses);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
	return 0;
}