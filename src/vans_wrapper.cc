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

    auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk)
    {
        vector<struct PENDING_REQUESTS>::iterator ptr;

        for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
            if (ait::translate_to_block_addr(ptr->request->address) == ait::translate_to_block_addr(logic_addr) )
                ptr->completed = true;

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
    else return -2;

};

int NVDIMM::add_wq( PACKET* packet)
{
    // simply drop write requests before the warmup
    if (all_warmup_complete < NUM_CPUS)
        return -1;

    logic_addr_t req_addr = packet->address;
    base_request_type req_type = base_request_type::write;
    clk_t curr_clk = current_core_cycle[packet->cpu];

    /*
    critical_read_callback under critical_load (see trace.cc).
    critical_load is when WQ is full. This check will be done from LLC itself.
    */
    auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk)
    {
    };

    base_request req( req_type, req_addr, curr_clk, callback);
    req.operation = 0;

    auto [issued, deterministic, next_clk, val] = model->issue_request(req);

    // add to outstanding requests
    if (issued) return -1;
    else return -2;
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
    clk_t curr_clk = current_core_cycle[cpu];
    model->tick(curr_clk);

    vector<struct PENDING_REQUESTS>::iterator ptr;

    for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
        if (ptr->completed == true)
        {
            if (ptr->request->instruction)
                upper_level_icache[ptr->request->cpu]->return_data(ptr->request);
            if (ptr->request->is_data)
                upper_level_dcache[ptr->request->cpu]->return_data(ptr->request);

            outstanding.erase(ptr);
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
        model->tick(current_core_cycle[cpu]);
        current_core_cycle[cpu]++;
    }

    return;
}

void NVDIMM::print_stats(void)
{
    model->print_counters();
    return;
}
