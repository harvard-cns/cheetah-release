# Cheetah

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


## Related publications 
* Cheetah: Accelerating Database Queries with Switch Pruning (ACM SIGMOD 2020) \[[link](https://dl.acm.org/doi/10.1145/3318464.3389698)\]
* Extended version (Arxiv 2020) \[[link](https://arxiv.org/abs/2004.05076)\]
* Poster (ACM SIGCOMM 2019) \[[link](https://dl.acm.org/doi/10.1145/3342280.3342311)\]

## Repository structure
The code in this repository is classified into three categories.

### Data Plane
The `data_plane` directory contains `code designed for Intel's [Barefoot Tofino](https://barefootnetworks.com/products/brief-tofino/) programmable switch. The code is written in a Tofino-specific variant of the P4-14 programming language. See [here](https://p4.org/) for more information on standard P4. 

### Control Plane
The `control_plane` directory contains the control plane rules you need to install to enable pruning for the queries implemented in the data plane. They are in a markdown file with separate sections for each query. The rules in each section are intended to be installed in the same order they are presented.

### Host
The `host` directory contains code used to serialize / deserialize any given list of values into a cheetah packet. This code is generic and does not make any assumptions regarding the query engine you are using. You can make it work with any kind of query engine or key value store as long as you write some patchwork code to allow Cheetah's packet serializer / deserializer to understand the file format(s) of the system you are integrating Cheetah with. Cheetah's packet serializer and deserializer is optimized using Intel's Dataplane Development Kit ([DPDK](https://www.dpdk.org/)). Therefore, you can only run this code on a DPDK-compliant NIC. For our evaluation, we used version 18.11 of Intel's DPDK along with Mellanox NICs.


## Deploying Cheetah

You need a proprietary compiler, the P4 compiler that is packaged with Intel's Barefoot Software Development Environment, to compile this code regardless of whether or not you intend to run it on real hardware. This is why we have chosen to release the code under the MIT License instead of one of the GNU GPL family of licenses ([relevant](https://softwareengineering.stackexchange.com/questions/318503/can-i-release-software-under-the-gpl-if-it-must-be-built-with-a-proprietary-comp)).

To deploy our implementation on hardware, we recommend using the first generation of Intel's Barefoot Tofino switches. We did not test Cheetah on the more recent Intel Barefoot Tofino 2 chip and do not know if our implementation of Cheetah is compatible with it. **Note that we have not included code required for TCP/IP forwarding and match-action table placement in our P4 scripts**. You will need to add your switch deployment's implementation TCP/IPF forwarding to the P4 scripts before you can deploy our implementation. Depending on your hardware, you may also need to add some code related to match-action table placement in order to be conformant with switch constraints (discussed in our ACM SIGMOD publication and Arxiv report).

You need to define the following constants based on your hardware: 

- For DISTINCT, `LRU_WIDTH`
- For GROUP-BY and JOIN, `REGISTER_COUNT_MAX`
- For TOP-N, `CHEETAH_TOPN_INT_MAX`

You also need to define the following constants based on the particular query you are optimizing:
- For TOP-N, `CHEETAH_TOPN_VALUE`
- For FILTERING, `FILTERING_CUTOFF`

## Tuning queries

We have included the default implementation of Cheetah's queries. However, to get optimal performance, you need to tune these queries based on your hardware constraints, the workload you are running, the query engine you are using and other factors. Here is a (not necessarily complete) set of knobs you can tune in our dataplane implementation. Note that these will also require (fairly intuitive) modifications / additions to control plane rules.

- For DISTINCT, you can increase or decrease the number of LRU caches (see the `cheetah_lru_import` files) used or assign more memory to each cache (tune `LRU_WIDTH`)
- For JOIN, you can increase or decrease the number of bloom filter blocks (see the definition for `CREATE_BLOOM`) or the memory assigned for each block (tune `REGISTER_COUNT_MAX`)


