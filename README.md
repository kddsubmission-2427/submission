# Two-Phase Partitioner

The implementation of Two-Phase Edge Partitioner (2PS). 

In this repo, we also implemented the below methods and a tool in C++.
* High Degrees Replicated First (HDRF) - Based on the following [publication](http://midlab.diag.uniroma1.it/articoli/PQDKI15CIKM.pdf): F. Petroni, L. Querzoni, G. Iacoboni, K. Daudjee and S. Kamali: "Hdrf: Efficient stream-based partitioning for power-law graphs". CIKM, 2015.
* Degree-Based Hashing (DBH) - Based on the following [publication](http://papers.nips.cc/paper/5396-distributed-power-law-graph-computing-theoretical-and-empirical-analysis.pdf): C. Xie, L. Yan, W. Li and Z. Zhang: "Distributed Power-law Graph Computing:
Theoretical and Empirical Analysis". NIPS, 2014
* Modularity Calculator - Based on the following [publication](https://www.pnas.org/content/103/23/8577): M. E. J. Newman: "Modularity and community structure in networks" PNAS, 2006 

## Installation

We tested the programs (``twophasepartitioner``, ``dbh``, ``modularity``) on Ubuntu 20.04 LTS and Debian 10.0.

The programs require the below packages: `g++`, `cmake`, `glog`, `gflags`, `boost`:
```
sudo apt-get install libgoogle-glog-dev libgflags-dev libboost-all-dev cmake g++
```

## Build Programs
```
cd twophasepartitioner
mkdir build && cd build
cmake ..
make
```
### Optimized Compilation

2PS and HDRF methods store the states of the vertices in a vertex-to-partition table. To optimize memory usage, we can define the number of partitions when the programs are built and allocate only the required amount of memory. The above build process will allocate the memory for 256 partitions. While it is still okay to use the fewer number of partitions when running `twophasepartitioner`, you will see the below warning message due to using memory inefficient:
```
#warning MAX NUMBER OF PARTITIONS is defined to 256.
```

To avoid this warning, use the below build step instead of just `make `:
```
make CXX_FLAGS="-Wall -std=c++14 -mcx16 -DSTATS -O3 -DNDEBUG -DMAX_NUM_PARTITION=<THE NUMBER OF PARTITIONS>"
```
Please change `<THE NUMBER OF PARTITIONS>` with the number of partitions you need.

Note: You need to add all the required CXX_FLAGS since they override the existing flags in CMakeLists.txt

Note: `-DSTATS` flag helps to print the replication factor and balancing value out to the console for DBH partitioner. However, printing out stats increases run-time and memory usage. Therefore, please remove this flag if you benchmark run-time and memory usage of DBH.

## Usage

### Two Phase partitioners and HDRF

Parameters:
* `filename`: path to the edge list file.
* `p`: the number of partitions that the graph will be divided into.
* `prepartitioner_type`: name of the clustering algorithm as a pre-partitioner. streamcom or rabbit. default: streamcom.
    * Please note that to be able to use Rabbit Order clustering, you need to follow the instructions in [Rabbit Order Integration](#rabbit) section.

Optional:
* `lambda`: lambda value for HDRF. default: 1.
* `shuffle`: shuffle the dataset. default: `false`.
* `memsize`: memory size in MB. memsize is used as a chunk size while shuffling data and batch size while partitioning edges. default: 100
* `balance_ratio`: imbalance factor for ensuring that the largest partition does not exceed the expected ratio. default: 1.05
* `print_coms`: print clusters (communities) to a file in ascending order of vertex ids. default: false.
    * Please note that when you enable this flag, the program will stop after completing the clustering step.
* `coms_filename`: the path to the output file to store the clusters in the ascending order of the vertex ids. default: "".
* `str_iters`: the number of iterations to run streamcom clustering in the first phase: default: 2.

2PS Examples:
```
./twophasepartitioner -filename ../sample/input.txt -p 4 -prepartitioner_type streamcom

./twophasepartitioner -filename ../sample/input.txt -p 4 -shuffle false -memsize 100 -prepartitioner_type streamcom -balance_ratio 1.05 -str_iters 2 
```

HDRF example:
```
./twophasepartitioner -filename ../sample/input.txt -p 4 -lambda 1.1
```

### DBH

Parameters:
* `filename`: path to the edge list file.
* `p`: the number of partitions that the graph will be divided into.

Optional:
* `memsize`: memory size in MB. Memsize is used as a chunk size while shuffling and batch size while partitioning. default: 100

DBH example:
```
./dbh -filename ../sample/input.txt -p 4
```

Note: `twophasepartitioner` and `dbh` programs  uses converter library to convert a given input file to the binary format to increase the efficiency of read-time and rename the vertex ids to ascending order of numerical values starting from 0. You can find the converted file in the same directory as the original input file. The extension of the binary file is .binedgelist. You can also use binary files as an input to Modularity Calculator program.


### Modularity Calculator
Parameters:
* `input`: path to the binary edgelist file, .binedgelist extension.
* `coms`: path to the communities file. You can generate the communities using `twophasepartitioner` with `print_coms` and `coms_filename` flags. 

Example:
```
./modularity -input ../sample/snap-graphs/alpha1.8.txt.binedgelist -coms ../sample/snap-graphs/alpha1.8coms.txt 
```

### Further Notes w.r.t Implementations 

* We used the below repository as a reference when implementing HDRF and DBH in C++. The reposity is owned by the author(s) of HDRF paper.
    * Fabio Petroni, https://github.com/fabiopetroni/VGP
*  The implementation for converting input file to binary edge list (converter/*) is modification of the respective implementation in the below reposity. The reposity is owned by the author(s) of Neighborhood Expansion paper. 
    * ANSR Lab, [https://github.com/ansrlab/edgepart](https://github.com/ansrlab/edgepart)
 

### <a name="rabbit">Rabbit Order Integration</a> 

The official implementation of Rabbit Order does not expose the clustering method in their implementation but use it internally for re-ordering the vertices. To be able to use Rabbit Order clustering, we modified the source code to expose the generated clusters while respecting the volume caps of clusters. However, due to the licensing of the Rabbit Order implementation in its official [repository](https://github.com/araij/rabbit_order), we cannot add the modified source code to this repo but, it is possible to follow the below steps to add the repo and apply the required changes.

1. Add the Rabbit Order repository as a submodule and apply the required changes to `CMakeLists.txt` and `main.cpp` files:
```
    git am patch_files/add-submodule.patch
```
2. Add the Rabbit Order files from the respective repository to `rabbit_order` directory using `git submodule`:
```
    git submodule update --init
```
3. After running the above command, you will find the original Rabbit Order files in `rabbit_order` directory. To modify the files, go to the directory and apply the second patch file:
```
   cd rabbit_order && git am ../patch_files/modify-rabbit.patch
```

The Rabbit Order implementation requires the below libraries: `tcmalloc_minimal` and `numa`. You can install the libraries using the below command:
```
sudo apt-get install libgoogle-perftools-dev
```

Now, you can run `twophasepartitioner` program using Rabbit Order clustering in the first phase. Make sure to re-build the program using the steps mentioned above to apply the changes. To be able to use Rabbit Order as a clustering method, you can set `prepartitioner_type` to `rabbit`. The full commadline arguments will look like this:

```
./twophasepartitioner -filename ../sample/input.txt -p 4 -prepartitioner_type rabbit
```

Note: Please ignore the below message when applying the patch files. The error message illustrates that the author of the changes are not given.
```
fatal: empty ident name (for <>) not allowed. 
```