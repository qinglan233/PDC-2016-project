/*
	Simulated Annealing algorithm for Traveling Salesman Problem
	@@ baseline version: no parallel optimization, single thread
	
	Input: xxx.tsp file
	Output: optimal value (total distance)
			& solution route: permutation of {1, 2, ..., N}
*/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <sys/time.h>
#include <pthread.h>
#include <omp.h>
#define MAXITER 20		// Proposal 20 routes and then select the best one
#define THRESH1 0.1		// Threshold 1 for the strategy
#define THRESH2 0.89	// Threshold 2 for the strategy
#define RELAX 40000		// The times of relaxation of the same temperature
#define ALPHA 0.999		// Cooling rate
#define INITEMP 99.0	// Initial temperature
#define STOPTEMP 0.001	// Termination temperature
#define MAXLAST 3		// Stop if the tour length keeps unchanged for MAXLAST consecutive temperature
#define MAXN 1000		// only support N <= 1000
using namespace std;

float minTourDist = -1;		// The distance of shortest path
int *minTour = NULL;		// The shortest path
int N = 0;					// Number of cities
float dist[MAXN][MAXN] = {};	// The distance matrix, use (i-1) instead of i
float currLen[MAXITER]={};
int *currTour[MAXITER]={};
int nprocess = 1;
int globalIter = -1;	// global iteration count
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* load the data */
void loadFile(char* filename) {
	FILE *pf;

	pf = fopen(filename, "r");
	if (pf == NULL) {
		printf("Cannot open the file!\n");
		exit(1);
	}
	char buff[200];
	fscanf(pf, "NAME: %[^\n]s", buff);
	printf("%s\n", buff);
	fscanf(pf, "\nTYPE: TSP%[^\n]s", buff);
	printf("%s\n", buff);
	fscanf(pf, "\nCOMMENT: %[^\n]s", buff);
	printf("%s\n", buff);
	fscanf(pf, "\nDIMENSION: %d", &N);
	printf("The N is: %d\n", N);
	fscanf(pf, "\nEDGE_WEIGHT_TYPE: %[^\n]s", buff);
	printf("the type is: %s\n", buff);
	memset(dist, 0, sizeof(dist));
	if (strcmp(buff, "EUC_2D") == 0) {
		fscanf(pf, "\nNODE_COORD_SECTION");
		float nodeCoord[MAXN][2] = {};
		int nid;
		float xx, yy;
		for (int i = 0; i < N; ++i) {
			fscanf(pf, "\n%d %f %f", &nid, &xx, &yy);
			nodeCoord[i][0] = xx;
			nodeCoord[i][1] = yy;
		}
		float xi, yi, xj, yj;
		for (int i = 0; i < N; ++i) {
			for (int j = i + 1; j < N; ++j) {
				xi = nodeCoord[i][0];
				yi = nodeCoord[i][1];
				xj = nodeCoord[j][0];
				yj = nodeCoord[j][1];
				dist[i][j] = (float)sqrt((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj));
				dist[j][i] = dist[i][j];
			}
		}
	}
	else if (strcmp(buff, "EXPLICIT") == 0) {
		fscanf(pf, "\nEDGE_WEIGHT_FORMAT: %[^\n]s", buff);
		fscanf(pf, "\n%[^\n]s", buff);
		char *disps = strstr(buff, "DISPLAY_DATA_TYPE");
		if (disps != NULL) {
			fscanf(pf, "\nEDGE_WEIGHT_SECTION");
		}
		float weight;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j <= i; ++j) {
				fscanf(pf, "%f", &weight);
				dist[i][j] = weight;
				dist[j][i] = weight;
			}
		}
	}
	return;
}

/* Calculate the length of the tour */
float tourLen(int *tour) {
	if (tour == NULL) {
		printf("tour not exist!\n");
		return -1;
	}
	float cnt = 0;
	for (int i = 0; i < N - 1; ++i) {
		cnt += dist[tour[i]][tour[i+1]];
	}
	cnt += dist[tour[N-1]][tour[0]];
	return cnt;
}

/* the main simulated annealing function */
void saTSP(int* tour) {
	float currLen = tourLen(tour);
	float temperature = INITEMP;
	float lastLen = currLen;
	int contCnt = 0; // the continuous same length times
	while (temperature > STOPTEMP) {
		temperature *= ALPHA;
		/* stay in the same temperature for RELAX times */
		for (int i = 0; i < RELAX; ++i) {
			/* Proposal 1: Block Reverse between p and q */
			int p = rand()%N, q = rand()%N;
			// If will occur error if p=0 q=N-1...
			if (abs(p - q) == N-1) {
				q = rand()%(N-1);
				p = rand()%(N-2);
			}
			if (p == q) {
				q = (q + 2) % N;
			}
			if (p > q) {
				int tmp = p;
				p = q;
				q = tmp;
			}
			int p1 = (p - 1 + N) % N;
			int q1 = (q + 1) % N;
			int tp = tour[p], tq = tour[q], tp1 = tour[p1], tq1 = tour[q1];
			float delta = dist[tp][tq1] + dist[tp1][tq] - dist[tp][tp1] - dist[tq][tq1];

			/* whether to accept the change */
			if ((delta < 0) || ((delta > 0) && 
				(exp(-delta/temperature) > (float)rand()/RAND_MAX))) {
				currLen = currLen + delta;
				int mid = (q - p) >> 1;
				int tmp;
				for (int k = 0; k <= mid; ++k) {
					tmp = tour[p+k];
					tour[p+k] = tour[q-k];
					tour[q-k] = tmp;
				}
				//currLen = tourLen(tour);
			}

		}

		if (fabs(currLen - lastLen) < 1e-2) {
			contCnt += 1;
			if (contCnt >= MAXLAST) {
				//printf("unchanged for %d times1!\n", contCnt);
				break;
			}
		}
		else
			contCnt = 0;
		lastLen = currLen;
	}
	
	return;
}

void *routine(void *idx) {
	long tid = (long)idx;
	int *localMin = (int *)malloc(sizeof(int) * N);
	int *tour = currTour[tid];
	float localMinDist = -1;
	int localIter = -1;
	for (int i = 0; i < MAXITER; ++i) {
		pthread_mutex_lock(&mutex);
		globalIter++;
		localIter = globalIter;
		pthread_mutex_unlock(&mutex);
		if (localIter >= MAXITER) {
			break;
		}
		random_shuffle((int *)tour, (int *)tour + N);
		saTSP((int *)tour);
		int len = tourLen(tour);
		if ((len < localMinDist) || (localMinDist < 0)) {
			localMinDist = len;
			memcpy(localMin, tour, sizeof(int) * N);
		}
	}
	memcpy(tour, localMin, sizeof(int) * N);
	free(localMin);
	return NULL;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Please enter the filename!\n");
		return 0;
	}
	else {
		loadFile(argv[1]);
	}
	if (argc > 2) {
		nprocess = atoi(argv[2]);
		if (nprocess > N) {
			nprocess = N;
		}
	}
	struct timeval start, stop;
	gettimeofday(&start, NULL);
	srand(time(0));
	minTour = (int *)malloc(sizeof(int) * N);
	pthread_mutex_init(&mutex, NULL);

	/* create "nprocess" threads and work! */
	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocess);
	for (int i = 0; i < nprocess; ++i) {
		currTour[i] = (int *)malloc(sizeof(int) * N);
		for (int j = 0; j < N; ++j)
			currTour[i][j] = j;
		if (pthread_create(&threads[i], NULL, routine, (void *)i)) {
			fprintf(stderr, "Fail to create thread!\n");
			exit(1);
		}
	}

	/* join all the threads */
	void *status;
	for (int i = 0; i < nprocess; ++i) {
		if (pthread_join(threads[i], &status)) {
			fprintf(stderr, "Fail to create thread!\n");
			exit(1);
		}
		currLen[i] = tourLen(currTour[i]);
	}
	
	/* find the minimal answer */
	int minidx = 0;
	for (int i = 0; i < nprocess; ++i) {
		if ((currLen[i] < minTourDist) || (minTourDist < 0)) {
			minTourDist = currLen[i];
			minidx = i;
		}
	}
	for (int i = 0; i < N; ++i) {
		minTour[i] = currTour[minidx][i];
	}
	gettimeofday(&stop, NULL);

	// ------------- Print the result! -----------------
	int tottime = stop.tv_sec - start.tv_sec;
	int timemin = tottime / 60;
	int timesec = tottime % 60;
	printf("Total time usage: %d min %d sec. \n", timemin, timesec);
	printf("The shortest length is: %f\n And the tour is: \n", minTourDist);
	for (int i = 0; i < N; ++i) {
		printf("%d \n", minTour[i]+1);
	}
	free(minTour);
	for (int i = 0; i < nprocess; ++i)
		free(currTour[i]);
	pthread_mutex_destroy(&mutex);
	return 0;
}
