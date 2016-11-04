# GenConvNMI
Generalized Conventional Mutual Information (GenConvMI) - NMI for Overlapping clusters compatible with standard [NMI](http://www.cs.plu.edu/courses/csce436/art%202.pdf) value, pure C++ version producing a single executable.

The paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

This is a refined version of the original [gecmi](https://bitbucket.org/dsign/gecmi) with additional features and improved performance, fully automated build and without the reduced dependencies (the Pyhton wrapper is removed). This version evaluates both NMI and FNMI (optionally) considering overlaps.  
FNMI is the Fair NMI, see the paper [Is Normalized Mutual Information a Fair Measure for Comparing Community Detection Methods](http://ieeexplore.ieee.org/document/7403755/) by Alessia Amelio and Clara Pizzuti, ASONAM'15. However, FNMI is less meaningful and fair than the actual NMI, because this measure affects by the number of clusters much more than by the actual structure of clusters that should be evaluated.  
Implemented by Artem Lutov <artem@exascale.info>

## Content
- [Deployment](#deployment)
	- [Dependencies](#dependencies)
	- [Compilation](#compilation)
- [Usage](#usage)
- [Related Projects](#related-projects)

# Deployment
## Dependencies
For the *compilation*:
- [boost](http://www.boost.org/boost) >= v.1.47
- [itbb](http://threadingbuildingblocks.org/itbb) >= v.3.0
- g++ >= v.4.8

For the *brebuilt executables*:
- libtbb2:  `$ sudo apt-get install libtbb2`
- libboost_program_options v1.58:  `$ sudo apt-get install libboost-program-options1.58.0`
- libstdc++6: `$ sudo apt-get install libstdc++6`

## Compilation

Just execute `make`:
```
$ make release
```
Both release and debug builts are performed by default. [Codeblocks](http://www.codeblocks.org/) project is provided and can be used for the interactive build.

# Usage

The application uses files in CNL format:

```
# The comments start with '#' like this line
# Each non-commented line is a module(cluster, community) consisting of the the member nodes separated by space / tab
1
1 2
2
```
where each line corresponds to the network nodes forming the cluster (community, module).
The [original `gecmi` format](https://github.com/eXascaleInfolab/GenConvMI#executable) is also supported.
 > The format is identified automatically from the file header (or it's absence).  

To get the normalized mutual information considering overlaps of two clusterings, execute:

```
$ gecmi file1 file2
```

Execution Options:
```
-h [ --help ]              produce help message
--input arg                name of the input files
-s [ --sync ]              synchronize the node base, for example to fairly
													 evaluate against top K selected clusters that
													 cover only part of the original nodes
-f [ --fnmi ]              evaluate also FNMI
-r [ --risk ] arg (=0.01)  probability of value being outside
-e [ --error ] arg (=0.01) admissible error
```
If you want to tweak the precision, use the options `-e` and `-r`, to set the error and
the risk respectively. See the [paper](http://arxiv.org/abs/1202.0425) for the meaning of these concepts.  
If the node base of the specified files is different (for example you decided to take the ground-truth clustering as a subset of the top K largest clusters) then it can be synchronized using the `-s` option. I.e. the nodes not present in the ground-truth clusters (communities) will be removed (also as the empty resulting clusters). The exception is thrown if the synchronization is not possible (in case the node base was not just reduced, rather it was totally different).

# Related Projects
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).
- [Overlapping-NMI](https://github.com/eXascaleInfolab/Overlapping-NMI) - Another method of the NMI evaluation for the overlapping clusters (communities) that is not compatible with the standard NMI value unlike GenConvNMI, but much faster (about one order faster).

**Note:** Please, [star this project](https://github.com/eXascaleInfolab/GenConvNMI) if you use it.
