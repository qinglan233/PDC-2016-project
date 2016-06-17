#include <iostream>
#include <vector>
#include <algorithm>

#include "sa.hpp"
#include "../../utils/global.hpp"
#include "../../utils/Communicator.hpp"

using namespace std;

int TSP::n;
float TSP::dist[N][N];

int main() {
	init();
	int n = getNumWorkers();
	Communicator<TSP> communicator;

	barrier();
	fprintf(stderr, "Finished loading.\n");

	vector<int> seedCount(n);
	vector<pair<int, int>> origin(n);
	vector<int> target(n);
	vector<vector<pair<int, int>>> arrange(n);
	float temperature = INIT_TEMP;
	while (true) {
		temperature *= RATIO;
		communicator.gatherMaster(seedCount);
		int sum = 0;
		for (int i = 1; i < n; ++i) {
			sum += seedCount[i];
			origin[i] = make_pair(-seedCount[i], i);
		}

		sort(origin.begin(), origin.end());
		int average = sum / (n - 1);
		for (int i = 1; i < n; ++i) {
			target[i] = average;
		}
		for (int i = 1; i < sum % (n - 1) + 1; ++i) {
			++target[i];
		}
		int j = n - 1;
		for (int i = 1; i < n; ++i) {
			int u = origin[i].second;
			while (-origin[i].first > target[i]) {
				int v = origin[j].second;
				int delta = min(-origin[i].first - target[i], target[j] - (-origin[j].first));
				arrange[u].push_back(make_pair(v, delta));
				origin[i].first += delta;
				origin[j].first -= delta;
				if (-origin[j].first == target[j]) {
					--j;
				}
			}
		}
		communicator.scatterMaster(arrange);
		communicator.syncBuffer();
		
		if (sum == 0) {
			break;
		}
	}

	barrier();
	fprintf(stderr, "Finished computing.\n");

	vector<TSP> results(n);
	communicator.gatherMaster(results);
	int k = 1;
	for (int i = 1; i < (int)results.size(); ++i) {
		if (results[i].curLen < results[k].curLen) {
			k = i;
		}
	}
	results[k].output();

	finalize();
	return 0;
}
