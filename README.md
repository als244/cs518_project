# cs518_project
#### Andrew Sheinberg & Dongsheng Yang
#### May 7th, 2024

First-stab at an accelerator-native "memory layer" in the context of a sptatiotemporal sharing system.

The structure of this repo is incomplete: there are 3 different makefiles for the various backends. The general-flow of the project will live-on, but many of these files and data-structures will be cleaned up and the interfaces polished for production use. 

Basic functionality (example_client.c) tested and working correctly the various backends:

- Nvidia GPU:
	- Hardware: Nvidia 3090
	- Runtime: CUDA 12.4
	- Driver: Open Kernel Modules 550
	- OS: Ubuntu 22.04/Linux 6.5.0-28-generic
- AMD GPU:
	- Hardware: AMD 7900XT
	- Runtime: ROCm 6.1
	- Driver: amdgpu 6.1
	- OS: Ubuntu 22.04/Linux 6.5.0-28-generic
- Intel GPU:
	- Hardware: Intel Arc A770
	- Runtime: Level Zero 1.9.2
	- Driver: i915 24.1.11
	- OS: Ubuntu 22.04/Linux 6.5.0-28-generic 

Notes:

- In our on-going project (and the blog post description) we have altered the design for allocating physical memory. In this repo you see that all physical memory is created at system init time. These chunks are engulfed by the system in the same manner as traditional pages. However, this incurs significant overhead when doing "reads" because the backing mapping operations are constant with respect to chunk size, so it is preferable to allocate physical memory chunks as the page-aligned size of the logical chunk itself. The tradeoff is that this strategy incurs a longer put() latency due to calls to the driver to allocate memory. In our new scheme, we rely on tracking the count of free frames and the backend driver's physical memory manager more critically than in this repo. 

- The backend virtual memory management functions are benchmarked below. 
