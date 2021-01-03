<p align="center">
  <h1 align="center"> ChampSim-VANS </h1>
  <p> <b>ChampSim</b> is a trace-based simulator for a microarchitecture study. You can sign up to the public mailing list by sending an empty mail to champsim+subscribe@googlegroups.com. ChampSim is hosted at https://github.com/ChampSim/ChampSim</p>
  <p> <b>VANS</b>  is a cycle-level NVRAM simulator. Its performance is initially calibrated to match the Intel Optane Persistent Memory. But you can reconfigure VANS to model other NVRAM systems. VANS is hosted at https://github.com/TheNetAdmin/VANS.git. VANS is part of this <a href="https://cseweb.ucsd.edu/~ziw002/files/micro20-lens-vans.pdf">MICRO 2020 paper</a></p>
  <p><b>Champsim-VANS</b> integrates the cache hierarchy of ChampSim with VANS. Note that as of now, multi-core support is disabled.</p>


# Clone ChampSim-VANS repository
```
git clone https://github.com/alannair/ChampSim-VANS.git
```

# Compile

ChampSim takes six parameters to compile: Branch predictor, L1I prefetcher, L1D prefetcher, L2C prefetcher, LLC replacement policy, and the number of cores.
```
$ ./build_champsim.sh [BRANCH] [L1I_PREFETCHER] [L1D_PREFETCHER] [L2C_PREFETCHER] [LLC_PREFETCHER] [LLC_REPLACEMENT]
```
For example, `/build_champsim.sh hashed_perceptron next_line next_line spp_dev no lru` builds a single-core processor with hashed-perceptron branch predictor, next-line L1I/L1D prefetchers, spp-dev L2 prefetcher, no LLC prefetcher and the baseline LRU replacement policy for the LLC.


# Download Traces

ChampSim-VANS should be able to run any trace that runs on ChampSim.
Professor Daniel Jimenez at Texas A&M University kindly provided traces for DPC-3. Use the following script to download these traces (~20GB size and max simpoint only).
```
$ cd scripts
$ ./download_dpc3_traces.sh
```
IPC-1 traces can be downloaded from <a href="https://drive.google.com/file/d/1qs8t8-YWc7lLoYbjbH_d3lf1xdoYBznf/view">here</a>.

# Run simulation

Execute `run_champsim.sh` with proper input arguments. The default `TRACE_DIR` in `run_champsim.sh` is to be set to the file path to the directory containing the traces. The default `CONFIG_FILE` is set to the file path to the VANS config file.<br>

* Run simulation with `run_champsim.sh` script.

```
Usage: ./run_champsim.sh [BINARY] [N_WARM] [N_SIM] [TRACE] [OPTION]
$ ./run_champsim.sh hashed_perceptron-next_line-next_line-spp_dev-no-lru-1core 5 5 client_001.champsimtrace.xz

${BINARY}: ChampSim binary compiled by "build_champsim.sh" present in the bin folder (bimodal-no-no-lru-1core)
${N_WARM}: number of instructions for warmup in millions (1 million)
${N_SIM}:  number of instructinos for detailed simulation in millions (10 million)
${TRACE}: trace name (client_001.champsimtrace.xz)
${OPTION}: extra option for "-low_bandwidth" (src/main.cc)
```
Simulation results will be stored under `results/results_${N_SIM}M` as `${TRACE}-${BINARY}-${OPTION}.txt`.<br>

# Add your own branch predictor, data prefetchers, and replacement policy
**Copy an empty template**
```
$ cp branch/branch_predictor.cc prefetcher/mybranch.bpred
$ cp prefetcher/l1d_prefetcher.cc prefetcher/mypref.l1d_pref
$ cp prefetcher/l2c_prefetcher.cc prefetcher/mypref.l2c_pref
$ cp prefetcher/llc_prefetcher.cc prefetcher/mypref.llc_pref
$ cp replacement/llc_replacement.cc replacement/myrepl.llc_repl
```

**Work on your algorithms with your favorite text editor**
```
$ vim branch/mybranch.bpred
$ vim prefetcher/mypref.l1d_pref
$ vim prefetcher/mypref.l2c_pref
$ vim prefetcher/mypref.llc_pref
$ vim replacement/myrepl.llc_repl
```

**Compile and test**
```
$ ./build_champsim.sh mybranch mypref mypref mypref myrepl
$ ./run_champsim.sh mybranch-mypref-mypref-mypref-myrepl-1core 1 10 bzip2_183B
```

# How to create traces

We have included only 4 sample traces, taken from SPEC CPU 2006. These
traces are short (10 million instructions), and do not necessarily cover the range of behaviors your
replacement algorithm will likely see in the full competition trace list (not
included).  We STRONGLY recommend creating your own traces, covering
a wide variety of program types and behaviors.

The included Pin Tool champsim_tracer.cpp can be used to generate new traces.
We used Pin 3.2 (pin-3.2-81205-gcc-linux), and it may require
installing libdwarf.so, libelf.so, or other libraries, if you do not already
have them. Please refer to the <a href="https://software.intel.com/sites/landingpage/pintool/docs/81205/Pin/html/">Pin documentation</a> for working with Pin 3.2.

Get this version of Pin:
```
wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-3.2-81205-gcc-linux.tar.gz
```

**Use the Pin tool like this**
```
pin -t obj-intel64/champsim_tracer.so -- <your program here>
```

The tracer has three options you can set:
```
-o
Specify the output file for your trace.
The default is default_trace.champsim

-s <number>
Specify the number of instructions to skip in the program before tracing begins.
The default value is 0.

-t <number>
The number of instructions to trace, after -s instructions have been skipped.
The default value is 1,000,000.
```
For example, you could trace 200,000 instructions of the program ls, after
skipping the first 100,000 instructions, with this command:
```
pin -t obj/champsim_tracer.so -o traces/ls_trace.champsim -s 100000 -t 200000 -- ls
```
Traces created with the champsim_tracer.so are approximately 64 bytes per instruction,
but they generally compress down to less than a byte per instruction using xz compression.

# Evaluate Simulation

ChampSim measures the IPC (Instruction Per Cycle) value as a performance metric. <br>
There are some other useful metrics printed out at the end of simulation. <br>
The VANS statistics are printed as it is.

## Bibliography

```bibtex
@inproceedings{LENS-VANS,
  author={Zixuan Wang and Xiao Liu and Jian Yang and Theodore Michailidis and Steven Swanson and Jishen Zhao},
  booktitle={2020 53rd Annual IEEE/ACM International Symposium on Microarchitecture (MICRO)},
  title={Characterizing and Modeling Non-Volatile Memory Systems},
  year={2020},
  pages={496-508},
  doi={10.1109/MICRO50266.2020.00049}
}
```

## License

[![](https://img.shields.io/github/license/TheNetAdmin/VANS)](LICENSE)
