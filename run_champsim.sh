#!/bin/bash

if [ "$#" -lt 4 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./run_champsim.sh [BINARY] [N_WARM] [N_SIM] [TRACE] [OPTION]"
    exit 1
fi

TRACE_DIR=/home/alannair/Documents/ipc1_public
CONFIG_FILE=/home/alannair/Documents/ChampSim-VANS/config/vans.cfg
BINARY=${1}
N_WARM=${2}
N_SIM=${3}
TRACE=${4}
OPTION=${5}

# Sanity check
if [ -z $TRACE_DIR ] || [ ! -d "$TRACE_DIR" ] ; then
    echo "[ERROR] Cannot find a trace directory: $TRACE_DIR"
    exit 1
fi

if [ ! -f "bin/$BINARY" ] ; then
    echo "[ERROR] Cannot find a ChampSim binary: bin/$BINARY"
    exit 1
fi

re='^[0-9]+$'
if ! [[ $N_WARM =~ $re ]] || [ -z $N_WARM ] ; then
    echo "[ERROR]: Number of warmup instructions is NOT a number" >&2;
    exit 1
fi

re='^[0-9]+$'
if ! [[ $N_SIM =~ $re ]] || [ -z $N_SIM ] ; then
    echo "[ERROR]: Number of simulation instructions is NOT a number" >&2;
    exit 1
fi

if [ ! -f "$TRACE_DIR/$TRACE" ] ; then
    echo "[ERROR] Cannot find a trace file: $TRACE_DIR/$TRACE"
    exit 1
fi

mkdir -p results/results_${N_SIM}M
OUTFILE=results/results_${N_SIM}M/${TRACE}-${BINARY}${OPTION}.txt

(./bin/${BINARY} -warmup_instructions ${N_WARM}000000 -simulation_instructions ${N_SIM}000000 -config ${CONFIG_FILE} ${OPTION} -traces ${TRACE_DIR}/${TRACE}) &> ${OUTFILE}

echo -e "\nNVDIMM STATS\n" >> ${OUTFILE}
cat results/stats_0 >> ${OUTFILE}

# r -warmup_instructions 5000000 -simulation_instructions 5000000 -config /home/alannair/Documents/ChampSim-VANS/config/vans.cfg -traces /home/alannair/Documents/ipc1_public/client_002.champsimtrace.xz &> results/debug.txt
