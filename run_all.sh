
binary=$1

val=ipc
TRACE_DIR=/home/alannair/Documents/ipc1_public
CONFIG_FILE=/home/alannair/Documents/ChampSim-VANS/config/vans.cfg

n_warm=50
n_sim=50

tracelist=ipc_trace_list.txt

total_workloads=`wc ${tracelist} | awk '{print $1}'`

results_folder=results/${val}_results

mkdir -p ${results_folder}

if [ ! -f bin/${binary} ]; then
    exit 1
fi



for((i=1;i<=${total_workloads};i++));
do
        trace=`sed -n ''${i}'p' ${tracelist} | awk '{print $1}'`
        trace_length=`sed -n ''${i}'p' ${tracelist} | awk '{print $2}'`

        sim_instr=$(echo "${trace_length} - 50000000" | bc -l)
        echo $i
        # OPTION=${trace}.${sim_instr}.cond

        if [ ! -f ${results_folder}/${trace}-${binary}.txt ]; then
                echo $trace

                (./bin/${binary} -warmup_instructions ${n_warm}000000 -simulation_instructions ${sim_instr} -config ${CONFIG_FILE} -traces ${TRACE_DIR}/${trace}) &> ${results_folder}/${trace}-${binary}.txt || true
        fi

done
