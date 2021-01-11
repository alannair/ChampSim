#include "vans_wrapper.h"

using namespace std;
using namespace vans;

void NVDIMM::init( void)
{
    cout << "\nInitializing VANS for NVDIMM from config file "
        << config_filename << endl;
};

int NVDIMM::add_rq( PACKET* packet)
{
    // simply return read requests with dummy response before the warmup
    if (all_warmup_complete < NUM_CPUS)
    {
        if (packet->instruction)
            upper_level_icache[packet->cpu]->return_data(packet);
        if (packet->is_data)
            upper_level_dcache[packet->cpu]->return_data(packet);

        return -1;
    }

    logic_addr_t req_addr = packet->address;
    base_request_type req_type = base_request_type::read;
    clk_t curr_clk = current_core_cycle[packet->cpu];

    auto callback = [&, curr_clk](logic_addr_t logic_addr, clk_t clk)
    {
        vector<struct PENDING_REQUESTS>::iterator ptr;

        for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
            if (ait::translate_to_block_addr(ptr->request->address) == ait::translate_to_block_addr(logic_addr) )
            {
                ptr->completed = true;

                TOTAL_LATENCY[ptr->request->cpu] += clk - curr_clk;
                ++TOTALS[ptr->request->cpu];
                if (ptr->request->type == LOAD)
                {
                    LOAD_LATENCY[ptr->request->cpu] += clk - curr_clk;
                    ++LOADS[ptr->request->cpu];
                }
                else if (ptr->request->type == PREFETCH)
                {
                    PREFETCH_LATENCY[ptr->request->cpu] += clk - curr_clk;
                    ++PREFETCHES[ptr->request->cpu];
                }
                else if (ptr->request->type == RFO)
                {
                    RFO_LATENCY[ptr->request->cpu] += clk - curr_clk;
                    ++RFOS[ptr->request->cpu];
                }

                break;
            }

    };

    base_request req( req_type, req_addr, curr_clk, callback);

    auto [issued, deterministic, next_clk, val] = model->issue_request(req);

    // add to outstanding requests
    if (issued)
    {
        struct PENDING_REQUESTS *new_request = new struct PENDING_REQUESTS;
        PACKET *new_packet = new PACKET;
        *new_packet = *packet;
        new_request->completed = false;
        new_request->request = new_packet;

        outstanding.push_back(*new_request);

        return -1;
    }
    else
    {
        return -2;
    }

};

int NVDIMM::add_wq( PACKET* packet)
{
    // simply drop write requests before the warmup
    if (all_warmup_complete < NUM_CPUS)
        return -1;

    logic_addr_t req_addr = packet->address;
    base_request_type req_type = base_request_type::write;
    clk_t curr_clk = current_core_cycle[packet->cpu];
    uint32_t curr_cpu = packet->cpu;

    auto callback = [&, curr_cpu, curr_clk](logic_addr_t logic_addr, clk_t clk)
    {

        TOTAL_LATENCY[curr_cpu] += clk - curr_clk;
        ++TOTALS[curr_cpu];
        WRITE_LATENCY[curr_cpu] += clk - curr_clk;
        ++WRITES[curr_cpu];

    };

    base_request req( req_type, req_addr, curr_clk, callback);
    req.operation = 0;

    auto [issued, deterministic, next_clk, val] = model->issue_request(req);

    // add to outstanding requests
    if (issued)
    {
        return -1;
    }
    else
    {
        return -2;
    }
};

int NVDIMM::add_pq(PACKET *packet)
{
    return -1;
};

void NVDIMM::return_data(PACKET *packet)
{

};

/*
    get_occupancy and get_size are never called
*/
uint32_t NVDIMM::get_occupancy( uint8_t queue_type, uint64_t address)
{
    if (outstanding.size() >= 4) return 4;
    else
    return 0;
};

uint32_t NVDIMM::get_size( uint8_t queue_type, uint64_t address)
{
    return 4;
};

void NVDIMM::increment_WQ_FULL( uint64_t address)
{
    // check if this is needed
};

void NVDIMM::operate()
{
    clk_t curr_clk = current_core_cycle[0]; // any will do
    model->tick(curr_clk);

    vector<struct PENDING_REQUESTS>::iterator ptr;

    for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
    {
        if (ptr->completed == true)
        {
            if (ptr->request->instruction)
                upper_level_icache[ptr->request->cpu]->return_data(ptr->request);
            if (ptr->request->is_data)
                upper_level_dcache[ptr->request->cpu]->return_data(ptr->request);

            outstanding.erase(ptr);
        }
    }
};

void NVDIMM::printout(void)
{
    vector<struct PENDING_REQUESTS>::iterator ptr;

    cout << "Printing OUTSTANDING Requests Queue\n";

    for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
        cout << hex << ptr->request->address << " " << ptr->completed << endl;
}

void NVDIMM::drain(void)
{
    model->drain();

    while (model->pending())
    {
        model->tick(current_core_cycle[0]);
        current_core_cycle[0]++;
    }

    return;
}

void NVDIMM::print_stats(void)
{
    for (int i=0; i<NUM_CPUS; ++i)
    {
        TOTAL_LATENCY[NUM_CPUS] += TOTAL_LATENCY[i];
        LOAD_LATENCY[NUM_CPUS] += LOAD_LATENCY[i];
        WRITE_LATENCY[NUM_CPUS] += WRITE_LATENCY[i];
        PREFETCH_LATENCY[NUM_CPUS] += PREFETCH_LATENCY[i];
        RFO_LATENCY[NUM_CPUS] += RFO_LATENCY[i];

        TOTALS[NUM_CPUS] += TOTALS[i];
        LOADS[NUM_CPUS] += LOADS[i];
        WRITES[NUM_CPUS] += WRITES[i];
        PREFETCHES[NUM_CPUS] += PREFETCHES[i];
        RFOS[NUM_CPUS] += RFOS[i];
    }

    // cout << "\nOVERALL VANS STATS for all cpus "
    //
    //     << "\n TOTAL LATENCY: " << TOTAL_LATENCY[NUM_CPUS] << " \t REQUESTS: "
    //     << TOTALS[NUM_CPUS] << " \t AVG: "
    //     << (float)TOTAL_LATENCY[NUM_CPUS] / TOTALS[NUM_CPUS]
    //
    //     << "\n LOAD LATENCY: " << LOAD_LATENCY[NUM_CPUS] << " \t REQUESTS: "
    //     << LOADS[NUM_CPUS] << " \t AVG: "
    //     << (float)LOAD_LATENCY[NUM_CPUS] / LOADS[NUM_CPUS]
    //
    //     << "\n WRITE LATENCY: " << WRITE_LATENCY[NUM_CPUS] << " \t REQUESTS: "
    //     << WRITES[NUM_CPUS] << " \t AVG: "
    //     << (float)WRITE_LATENCY[NUM_CPUS] / WRITES[NUM_CPUS]
    //
    //     << "\n PREFETCH LATENCY: " << PREFETCH_LATENCY[NUM_CPUS] << " \t REQUESTS: "
    //     << PREFETCHES[NUM_CPUS] << " \t AVG: "
    //     << (float)PREFETCH_LATENCY[NUM_CPUS] / PREFETCHES[NUM_CPUS]
    //
    //     << "\n RFO LATENCY: " << RFO_LATENCY[NUM_CPUS] << " \t REQUESTS: "
    //     << RFOS[NUM_CPUS] << " \t AVG: "
    //     << (float)RFO_LATENCY[NUM_CPUS] / RFOS[NUM_CPUS]
    // 
    //     << endl;

    for (int i=0; i<NUM_CPUS; ++i)
    {
        /* 10^3 = 10^9 (tCK is in ns) / 10^6 (BW is in MBps)
         * Each request is for a 64 B cacheline
         */
        float total_bw = (64 * 1000 * (float)TOTALS[i]) / (ooo_cpu[i].finish_sim_cycle * t_CK);
        float load_bw = (64 * 1000 * (float)LOADS[i]) / (ooo_cpu[i].finish_sim_cycle * t_CK);
        float write_bw = (64 * 1000 * (float)WRITES[i]) / (ooo_cpu[i].finish_sim_cycle * t_CK);
        float prefetch_bw = (64 * 1000 * (float)PREFETCHES[i]) / (ooo_cpu[i].finish_sim_cycle * t_CK);
        float rfo_bw = (64 * 1000 * (float)RFOS[i]) / (ooo_cpu[i].finish_sim_cycle * t_CK);

        cout << "\nOVERALL VANS STATS for cpu " << i

            << " \n TOTAL REQUESTS: " << TOTALS[i] << " \t AVG LATENCY: "
            << (float)TOTAL_LATENCY[i] / TOTALS[i] << " cycles"
            << " \t Avg BW: " << total_bw << " MB/s"

            << " \n LOAD REQUESTS: " << LOADS[i] << " \t AVG LATENCY: "
            << (float)LOAD_LATENCY[i] / LOADS[i] << " cycles"
            << " \t Avg BW: " << load_bw << " MB/s"

            << " \n WRITE REQUESTS: " << WRITES[i] << " \t AVG LATENCY: "
            << (float)WRITE_LATENCY[i] / WRITES[i] << " cycles"
            << " \t Avg BW: " << write_bw << " MB/s"

            << " \n PREFETCH REQUESTS: " << PREFETCHES[i] << " \t AVG LATENCY: "
            << (float)PREFETCH_LATENCY[i] / PREFETCHES[i] << "cycles"
            << " \t Avg BW: " << prefetch_bw << " MB/s"

            << " \n RFO REQUESTS: " << RFOS[i] << " \t AVG LATENCY: "
            << (float)RFO_LATENCY[i] / RFOS[i] << " cycles"
            << " \t Avg BW: " << rfo_bw << " MB/s"

            << endl;
    }

    model->print_counters();
    return;
}

void NVDIMM::reset_stats(void)
{
    for (int i=0; i<=NUM_CPUS; ++i)
        TOTAL_LATENCY[i] = LOAD_LATENCY[i] = WRITE_LATENCY[i] = PREFETCH_LATENCY[i] = RFO_LATENCY[i] = TOTALS[i] = LOADS[i] = WRITES[i] = PREFETCHES[i] = RFOS[i] = 0;

    model->reset_counters();
    return;
}
