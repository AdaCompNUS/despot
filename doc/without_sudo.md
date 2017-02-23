# Usage without `sudo`

We provide makefiles to compile directly without CMake or `sudo` 

## Local Installation

Compile using make:
```bash
cd <despot_dir>
make
```

## Examples

Compile using make:
```bash
cd <despot_dir>examples/pomdpx_models
make

./pomdpx -m ../data/tag.pomdpx --runs 2 [OPTIONS]...
```