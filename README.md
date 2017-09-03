# GenConvNMI
Generalized Conventional Mutual Information (GenConvMI) - NMI for Overlapping clusters compatible with standard [NMI](http://www.cs.plu.edu/courses/csce436/art%202.pdf) value, pure C++ version producing a single executable.  
GenConvMI applicable to evaluate both *overlapping (crisp and fuzzy) and multi-resolution clustering*: a single collection can contain all these mixed clusters (communities, modules) and be correctly evaluated, which is a unique feature.

The paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

This implementation is part of the [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) benchmark. *GenConvNMI* is significantly reimplemented version of the original [gecmi](https://bitbucket.org/dsign/gecmi) with additional features, much better performance (~2 ORDERS faster, consumes 2x less memory and is more accurate on large networks than the original version), fully automated build and without the redundant dependencies (the Pyhton wrapper is removed). This version evaluates both NMI and FNMI (optionally) considering overlaps.  
FNMI is so called *Fair NMI*, see the paper [Is Normalized Mutual Information a Fair Measure for Comparing Community Detection Methods](http://ieeexplore.ieee.org/document/7403755/) by Alessia Amelio and Clara Pizzuti, ASONAM'15. However, FNMI is less meaningful and less fair than the standard NMI, because FNMI measure is affected by the number of clusters much more than by their actual structure that should be evaluated.  
Implemented by Artem Lutov <artem@exascale.info>

## Content
- [Deployment](#deployment)
	- [Requirements](#requirements)
	- [Compilation](#compilation)
- [Usage](#usage)
- [Related Projects](#related-projects)

# Deployment
## Requirements
For the *compilation*:
- [boost](http://www.boost.org/boost) >= v.1.47
- [itbb](http://threadingbuildingblocks.org/itbb) >= v.3.0 or *libtbb-dev*
- g++ >= v.5

For the *prebuilt executable* on Linux Ubuntu 16.04 x64:
- libtbb2:  `$ sudo apt-get install libtbb2`
- libboost_program_options v1.58:  `$ sudo apt-get install libboost-program-options1.58.0`
- libstdc++6: `$ sudo apt-get install libstdc++6`

## Compilation

Just execute `make`:
```
$ make release
```
Both release and debug builds are performed by default. [Codeblocks](http://www.codeblocks.org/) project is provided and can be used for the interactive build.

> Build errors might occur if the default *g++/gcc <= 5.x*.  
`g++-5` should be installed and `Makefile` might need to be edited replacing `g++`, `gcc` with `g++-5`, `gcc-5`.

# Usage

The application uses files in CNL format:

```
# The comments start with '#' like this line
# Each non-commented line is a module(cluster, community) consisting of the the member nodes separated by space / tab
1
1 2
2
```
where each line corresponds to the network nodes forming the cluster (community, module). Empty lines and comments (lines starting with #) are skipped.
> `:` symbol following the node id is used to specify the membership share in the CNL format, which is not supported by gecmi and is omitted (trimmed).

> All input ids must form the sequential range starting from 0 or 1!

To get the normalized mutual information considering overlaps of two clusterings, execute:

```
$ gecmi file1 file2
```

Execution Options:
```
  -h [ --help ]                produce help message
  --input arg                  name of the input files
  -s [ --sync ]                synchronize the node base, for example to fairly
                               evaluate against top K selected clusters that 
                               are subset of the original nodes
  -n [ --nmis ]                output both NMI [max] and NMI_sqrt
  -f [ --fnmi ]                evaluate also FNMI, includes '-x'
  -r [ --risk ] arg (=0.01)    probability of value being outside
  -e [ --error ] arg (=0.01)   admissible error
  -a [ --fast ]                apply fast approximate evaluations that are less
                               accurate, but much faster on large networks
  -m [ --membership ] arg (=1) average expected membership of nodes in the 
                               clusters, > 0, typically >= 1
```
If you want to tweak the precision, use the options `-e` and `-r`, to set the error and
the risk respectively. See the [paper](http://arxiv.org/abs/1202.0425) for the meaning of these concepts.  
If the node base of the specified files is different (for example you decided to take the ground-truth clustering as a subset of the top K largest clusters) then it can be synchronized using the `-s` option. I.e. the nodes not present in the ground-truth clusters (communities) will be removed (also as the empty resulting clusters). The exception is thrown if the synchronization is not possible (in case the node base was not just reduced, rather it was totally different).

# Related Projects
- [xmeasures](https://github.com/eXascaleInfolab/xmeasures)  - Extrinsic clustering measures evaluation for the multi-resolution clustering with overlaps (covers): F1_gm for overlapping multi-resolution clusterings with possible unequal node base and standard NMI for non-overlapping clustering on a single resolution.
- [OvpNMI](https://github.com/eXascaleInfolab/OvpNMI) - Another method of the NMI evaluation for the overlapping clusters (communities) that is not compatible with the standard NMI value unlike GenConvNMI, but it is much faster than GenConvNMI.
- [resmerge](https://github.com/eXascaleInfolab/resmerge)  - Resolution levels clustering merger with filtering. Flattens hierarchy/list of multiple resolutions levels (clusterings) into the single flat clustering with clusters on various resolution levels synchronizing the node base.
- [ExecTime](https://bitbucket.org/lumais/exectime/)  - A lightweight resource consumption profiler.
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).

**Note:** Please, [star this project](https://github.com/eXascaleInfolab/GenConvNMI) if you use it.
