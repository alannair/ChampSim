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

    // check if block-alignment is really needed
    // logic_addr_t req_addr = ait::translate_to_block_addr(packet->address);
    logic_addr_t req_addr = packet->address;
    base_request_type req_type = base_request_type::read;
    clk_t curr_clk = current_core_cycle[packet->cpu];

    /*
    critical_read_callback under critical_load (see trace.cc).
    critical_load is when RQ is full. This check will be done from LLC itself.
    */
    auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk)
    {
        // cout << "READ CALLBACK: " << dec << curr_clk << "\t" << hex << logic_addr << "\n";
        vector<struct PENDING_REQUESTS>::iterator ptr;

        for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
            if (ait::translate_to_block_addr(ptr->request->address) == ait::translate_to_block_addr(logic_addr) )
                ptr->completed = true;

        // this->printout();
    };

    base_request req( req_type, req_addr, curr_clk, callback);

    auto [issued, deterministic, next_clk, val] = model->issue_request(req);

    // cout << issued << "\tREAD\t" << dec << curr_clk << "\t" << hex << req_addr << endl;
    // cout << "Done\n";
    // this->printout();

    // if (val != -1)
    // {
    //     if (val == -2)
    //     {
    //         // found a writeback
    //         if (packet->instruction)
    //             upper_level_icache[packet->cpu]->return_data(packet);
    //         if (packet->is_data)
    //             upper_level_dcache[packet->cpu]->return_data(packet);
    //     }
    //     else if (val >= 0)
    //     {
    //         // merged with another RQ entry at index val
    //         return val;
    //     }
    // }
    // else
    // {
    //     // add to outstanding requests
    //     struct PENDING_REQUESTS *new_request = new struct PENDING_REQUESTS;
    //     PACKET *new_packet = new PACKET;
    //     *new_packet = *packet;
    //     new_request->completed = false;
    //     new_request->request = new_packet;
    //
    //     outstanding.push_back(*new_request);
    //     printf("SECONF\n");
    //     this->printout();
    //     printf("\n");
    // }
    //
    // TODO: update_schedule_cycle


    // add to outstanding requests
    if (issued)
    {
        struct PENDING_REQUESTS *new_request = new struct PENDING_REQUESTS;
        PACKET *new_packet = new PACKET;
        *new_packet = *packet;
        new_request->completed = false;
        new_request->request = new_packet;

        outstanding.push_back(*new_request);
        // printf("SECONF\n");
        // this->printout();
        // printf("\n");

        return -1;
    }
    else return -2;

};

int NVDIMM::add_wq( PACKET* packet)
{
    // simply drop write requests before the warmup
    if (all_warmup_complete < NUM_CPUS)
        return -1;

    // check if block-alignment is really needed
    // logic_addr_t req_addr = ait::translate_to_block_addr(packet->address);
    logic_addr_t req_addr = packet->address;
    base_request_type req_type = base_request_type::write;
    clk_t curr_clk = current_core_cycle[packet->cpu];

    /*
    critical_read_callback under critical_load (see trace.cc).
    critical_load is when WQ is full. This check will be done from LLC itself.
    */
    auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk)
    {
        // cout << "WRITE CALLBACK: " << dec << curr_clk << "\t" << hex << logic_addr << "\n";
        // vector<struct PENDING_REQUESTS>::iterator ptr;
        //
        // for (ptr = outstanding.begin(); ptr < outstanding.end(); ptr++)
        //     if (ait::translate_to_block_addr(ptr->request->address) == ait::translate_to_block_addr(logic_addr) )
        //         ptr->completed = true;
    };

    base_request req( req_type, req_addr, curr_clk, callback);
    req.operation = 0;

    auto [issued, deterministic, next_clk, val] = model->issue_request(req);

    // cout << issued << "\tWRITE\t" << dec << curr_clk << "\t" << hex << req_addr << endl;

    // cout << issued << " " << val << " " << hex << req_addr << "\n";
    // this->printout();
    // cout << "Done\n";

    // if (val >= 0)
    // {
    //     // merged with another WQ entry at index val
    //     return val;
    // }
    // else
    // {
    //     // add to outstanding requests
    //     struct PENDING_REQUESTS *new_request = new struct PENDING_REQUESTS;
    //     PACKET *new_packet = new PACKET;
    //     *new_packet = *packet;
    //     new_request->completed = false;
    //     new_request->request = new_packet;
    //
    //     outstanding.push_back(*new_request);
    //     printf("SECONFWWWWW\n");
    //     this->printout();
    //     printf("\n");
    // }

    // add to outstanding requests
    if (issued)
    {
        // struct PENDING_REQUESTS *new_request = new struct PENDING_REQUESTS;
        // PACKET *new_packet = new PACKET;
        // *new_packet = *packet;
        // new_request->completed = false;
        // new_request->request = new_packet;
        //
        // outstanding.push_back(*new_request);
        // printf("SECONF\n");
        // this->printout();
        // printf("\n");

        return -1;
    }
    else return -2;

    // TODO: update_schedule_cycle

    // return -1;
};

int NVDIMM::add_pq(PACKET *packet)
{
    return -1; // we don't do that here
};

void NVDIMM::return_data(PACKET *packet)
{

};

uint32_t NVDIMM::get_occupancy( uint8_t queue_type, uint64_t address)
{
    // logic_addr_t req_addr = address; //ait::translate_to_block_addr(address);
    // clk_t curr_clk = clk_invalid;
    // base_request_type req_type;
    //
    // if (queue_type == 1)
    //     req_type = base_request_type::read;
    // else if (queue_type == 2)
    //     req_type = base_request_type::write;
    // else return 0;
    //
    // auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk) {};
    //
    // base_request req( req_type, req_addr, curr_clk, callback);
    // req.operation = 2; // see request_queue.h
    //
    // auto [issued, deterministic, next_clk, val] = model->issue_request(req);
    //
    // if (all_warmup_complete >= NUM_CPUS)
    //     printf("GET OCC %d\n", val);
    //
    // return val;
    if (outstanding.size() >= 4) return 4;
    else
    return 0;
};

uint32_t NVDIMM::get_size( uint8_t queue_type, uint64_t address)
{
    // logic_addr_t req_addr = address; //ait::translate_to_block_addr(address);
    // clk_t curr_clk = clk_invalid;
    // base_request_type req_type;
    //
    // if (queue_type == 1)
    //     req_type = base_request_type::read;
    // else if (queue_type == 2)
    //     req_type = base_request_type::write;
    // else return 0;
    //
    // auto callback = [&](logic_addr_t logic_addr, clk_t curr_clk) {};
    //
    // base_request req( req_type, req_addr, curr_clk, callback);
    // req.operation = 1; // see request_queue.h
    //
    // auto [issued, deterministic, next_clk, val] = model->issue_request(req);
    //
    // if (all_warmup_complete >= NUM_CPUS)
    //     printf("GET SZ %d\n", val);
    //
    // return val;
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

    // if (count != outstanding.size())
    // {
    //     cout << "\n OPERATE: " << count << "\n";
    //     printout();
    //     count = outstanding.size();
    // }

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
