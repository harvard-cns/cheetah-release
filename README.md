# Cheetah

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


## Related publications 
* Cheetah: Accelerating Database Queries with Switch Pruning (ACM SIGMOD 2020) \[[link](https://dl.acm.org/doi/10.1145/3318464.3389698)\]
* Extended version (Arxiv 2020) \[[link](https://arxiv.org/abs/2004.05076)\]
* Poster (ACM SIGCOMM 2019) \[[link](https://dl.acm.org/doi/10.1145/3342280.3342311)\]

## Repository structure
The code in this repository is classified into three categories.

### Data Plane
The `data\_plane` directory contains `code designed for Intel's [Barefoot Tofino](https://barefootnetworks.com/products/brief-tofino/) programmable switch. The code is written in a Tofino-specific variant of the P4-14 programming language. See [here](https://p4.org/) for more information on standard P4. 

### Control Plane
The `control\_plane` directory contains the control plane rules you need to install to enable pruning for the queries implemented in the data plane. They are in a markdown file with separate sections for each query. The rules in each section are intended to be installed in the same order they are presented.

### Host
The `host` directory contains code used to serialize / deserialize any given list of values into a cheetah packet. This code is generic and does not make any assumptions regarding the query engine you are using. You can make it work with any kind of query engine or key value store as long as you write some patchwork code to allow Cheetah's packet serializer / deserializer to understand the file format(s) of the system you are integrating Cheetah with.

You need a proprietary compiler, the P4 compiler that is packaged with Intel's Barefoot Software Development Environment, to compile this code 
