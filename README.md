# PDC course project 
### Parallel Heuristic Algorithms for TSP
============

TSP Problem:  
The traveling salesman problem (TSP) asks the following question: 
Given a list of cities and the distances between each pair of cities, what is the shortest possible route that visits each city exactly once and returns to the origin city?

#### Datasets (you can find the data in "dataset" folder):  
1. [TSP Data for the Traveling Salesperson Problem](https://people.sc.fsu.edu/~jburkardt/datasets/tsp/tsp.html)  
    All of the data are saved in adjacent matrix format.  
    DANTZIG42 is a set of 42 cities, from TSPLIB. The minimal tour has length 699.  
    FRI26 is a set of 26 cities, from TSPLIB. The minimal tour has length 937.  
    GR17 is a set of 17 cities, from TSPLIB. The minimal tour has length 2085.  
2. [TSPLIB library](http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp/)  
    ch150 is a set of 150 cities, from TSPLIB. The minimal tour has length 6528.
3. [China - 71,009 Cities](http://www.math.uwaterloo.ca/tsp/world/chlog.html)  
    It contains China - 71,009 Cities.  

#### Folders
1. dataset/: ALL of the datasets
2. baseline/: our baseline codes
3. distributed/: our MPI distributed version of codes
4. parallel/: our parallel version of codes

#### TODO list:
- [x] Find dataset for TSP
- [ ] Write baseline (single thread version) for Genetic algorithm (GA)
- [ ] Write baseline (single thread versiion) for Simulated Annealing algorithm (SA)
- [ ] Build MPI environment on the server
- [ ] Write MPI version of two algorithms
- [ ] Write CUDA / OpenCL version of two algorithms
- [ ] **Optimization for MPI and GPU = =
- [ ] Test the performance of the baseline and the parallel versions.
