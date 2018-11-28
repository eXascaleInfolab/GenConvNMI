# GenConvNMI
Generalized Conventional Mutual Information (GenConvMI) - NMI for Overlapping clusters compatible with standard [NMI](http://www.cs.plu.edu/courses/csce436/art%202.pdf) value, pure C++ version producing a single executable.  
GenConvMI applicable to evaluate both *overlapping (crisp and fuzzy) and multi-resolution clustering*: a single collection can contain all these mixed clusters (communities, modules) and be correctly evaluated, which is a unique feature. Moreover, the evaluating collections may contain duplicated clusters, which is useful for the semantic types evaluation (when differently named types actually have the same members, i.e. equivalent types exist).

The original paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

The paper describing implemented extensions: "Accuracy Evaluation of Overlapping and Multi-resolution Clustering Algorithms on Large Datasets" by Artem Lutov, Mourad Khayati and Philippe
Cudré-Mauroux, 2018

This implementation is part of the [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) benchmark. *GenConvNMI* is significantly reimplemented version of the original [gecmi](https://bitbucket.org/dsign/gecmi) with additional features, much better performance (~2 ORDERS faster, consumes 2x less memory and is more accurate on large networks than the original version), duplicated clusters support added, fully automated build and without the redundant dependencies (the Pyhton wrapper is removed) and more... This version evaluates both NMI and FNMI (optionally) considering overlaps.  
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
If all input ids are not form the sequential range starting from 0 or 1 then `-i` option **should be used** to remap the input ids before the evaluation.

To get the normalized mutual information considering overlaps of two clusterings, execute:

```
$ gecmi file1 file2
```

Execution Options:
```
  -h [ --help ]                produce help message
  --input arg                  name of the input files
  -s [ --sync ] arg            synchronize the node base omitting the 
                               non-matching nodes
                               NOTE: The node base is either the first input 
                               file or '-' (automatic selection of the input 
                               file having the least number of nodes)
  -i [ --id-remap ]            remap ids allowing arbitrary input ids 
                               (non-contiguous ranges), otherwise ids should 
                               form a solid range and start from 0 or 1
  -n [ --nmis ]                output both NMI [max] and NMI_sqrt
  -f [ --fnmi ]                evaluate also FNMI, includes '-n'
  -r [ --risk ] arg (=0.01)    probability of value being outside
  -e [ --error ] arg (=0.01)   admissible error
  -a [ --fast ]                apply fast approximate evaluations that are less
                               accurate, but much faster on large networks
  -m [ --membership ] arg (=1) average expected membership of nodes in the 
                               clusters, > 0, typically >= 1
  -d [ --retain-dups ]         retain duplicated clusters if any instead of 
                               filtering them out (not recommended)
```
If you want to tweak the precision, use the options `-e` and `-r`, to set the error and
the risk respectively. See the [paper](http://arxiv.org/abs/1202.0425) for the meaning of these concepts.  
If the node base of the specified files is different (for example you decided to take the ground-truth clustering as a subset of the top K largest clusters) then it can be synchronized using the `-s` option. I.e. the nodes not present in the ground-truth clusters (communities) will be removed (also as the empty resulting clusters). The exception is thrown if the synchronization is not possible (in case the node base was not just reduced, rather it was totally different).

**Note:** Please, [star this project](https://github.com/eXascaleInfolab/GenConvNMI) if you use it.

# Related Projects
- [xmeasures](https://github.com/eXascaleInfolab/xmeasures)  - Extrinsic quality (accuracy) measures evaluation for the overlapping clustering on large datasets: family of mean F1-Score (including clusters labeling), Omega Index (fuzzy version of the Adjusted Rand Index) and standard NMI (for non-overlapping clusters).
- [OvpNMI](https://github.com/eXascaleInfolab/OvpNMI) - Another method of the NMI evaluation for the overlapping clusters (communities) that is not compatible with the standard NMI value unlike GenConvNMI, but it is much faster than GenConvNMI.
- [Clubmark](https://github.com/eXascaleInfolab/clubmark) - A parallel isolation framework for benchmarking and profiling clustering (community detection) algorithms considering overlaps (covers).
- [resmerge](https://github.com/eXascaleInfolab/resmerge)  - Resolution levels clustering merger with filtering. Flattens hierarchy/list of multiple resolutions levels (clusterings) into the single flat clustering with clusters on various resolution levels synchronizing the node base.
- [ExecTime](https://bitbucket.org/lumais/exectime/)  - A lightweight resource consumption profiler.
- [TInfES](https://github.com/eXascaleInfolab/TInfES)  - Type inference evaluation scripts and accessory apps used for the benchmarking.
