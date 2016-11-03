# GenConvNMI
Generalized Conventional Mutual Information (GenConvMI) - NMI for Overlapping clusters compatible with standard NMI, pure C++ version producing a single executable.

The paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

This is a refined version of the original [gecmi](https://bitbucket.org/dsign/gecmi) with additional features, ~1.5 times better performance, fully automated build and without the reduced dependencies (the Pyhton wrapper is removed).  
Modified and extended by Artem Lutov <artem@exascale.info>

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
*The format is identified automatically from the file header (or it's absence).*

To get the normalized mutual information considering overlaps of two clusterings, execute:

```
$ gecmi file1 file2
```

If you want to tweak the precision, use the options `-e` and `-r`, to set the error and
the risk respectively.  
See the [paper](http://arxiv.org/abs/1202.0425) for the meaning of these concepts.

# Related Projects
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).

**Note:** Please, [star this project](https://github.com/eXascaleInfolab/GenConvNMI) if you use it.
