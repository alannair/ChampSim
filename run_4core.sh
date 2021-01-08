#!/bin/bash

if [ "$#" -lt 8 ] || [ "$#" -gt 9 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./run_4core.sh [BINARY] [N_WARM] [N_SIM] [N_MIX] [TRACE0] [TRACE1] [TRACE2] [TRACE3] [OPTION]"
    exit 1
fi

TRACE_DIR=/home/alannair/Documents/ipc1_public
CONFIG_FILE=/home/alannair/Documents/ChampSim-VANS/config/vans.cfg
BINARY=${1}
N_WARM=${2}
N_SIM=${3}
N_MIX=${4}
TRACE0=${5}
TRACE1=${6}
TRACE2=${7}
TRACE3=${8}
OPTION=${9}

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

if [ ! -f "$TRACE_DIR/$TRACE0" ] ; then
    echo "[ERROR] Cannot find a trace0 file: $TRACE_DIR/$TRACE0"
    exit 1
fi

if [ ! -f "$TRACE_DIR/$TRACE1" ] ; then
    echo "[ERROR] Cannot find a trace1 file: $TRACE_DIR/$TRACE1"
    exit 1
fi

if [ ! -f "$TRACE_DIR/$TRACE2" ] ; then
    echo "[ERROR] Cannot find a trace2 file: $TRACE_DIR/$TRACE2"
    exit 1
fi

if [ ! -f "$TRACE_DIR/$TRACE3" ] ; then
    echo "[ERROR] Cannot find a trace3 file: $TRACE_DIR/$TRACE3"
    exit 1
fi

mkdir -p results/results_4core_${N_SIM}M
OUTFILE=results/results_4core_${N_SIM}M/${N_MIX}-${BINARY}${OPTION}.txt

(./bin/${BINARY} -warmup_instructions ${N_WARM}000000 -simulation_instructions ${N_SIM}000000 ${N_MIX} -config ${CONFIG_FILE} ${OPTION} -traces ${TRACE_DIR}/${TRACE0} ${TRACE_DIR}/${TRACE1} ${TRACE_DIR}/${TRACE2} ${TRACE_DIR}/${TRACE3}) &> ${OUTFILE}

# r -warmup_instructions 5000000 -simulation_instructions 5000000 test1234 -config /home/alannair/Documents/ChampSim-VANS/config/vans.cfg -traces /home/alannair/Documents/ipc1_public/client_001.champsimtrace.xz /home/alannair/Documents/ipc1_public/client_002.champsimtrace.xz /home/alannair/Documents/ipc1_public/client_003.champsimtrace.xz /home/alannair/Documents/ipc1_public/client_004.champsimtrace.xz
