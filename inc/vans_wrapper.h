#include <iostream>
#include <string>
#include <unistd.h>
#include "memory_class.h"
#include "config.h"
#include "factory.h"
#include "trace.h"

using namespace std;

struct PENDING_REQUESTS
{
    bool completed;
    PACKET *request;
    int x;
};

class NVDIMM : public MEMORY
{
public:
    string config_filename;
    std::shared_ptr<vans::base_component> model;
    int fill_level;
    int cpu; // a single-core machine assumed
    vector<struct PENDING_REQUESTS> outstanding;
    int count;


    NVDIMM(string filename, int fill) :
     config_filename(filename), fill_level(fill)
    {
        cpu = 0;
        count = 0;
        // upper_level_dcache =
        auto cfg   = vans::root_config(config_filename);
        model = vans::factory::make(cfg);
    };

    ~NVDIMM()
    {};

    // methods
    void init (void);

    int add_rq (PACKET *packet); //done
    int add_wq (PACKET *packet); //done
    int add_pq (PACKET *packet); //done

    void return_data (PACKET *packet); //done
    void operate (void); //done
    void increment_WQ_FULL (uint64_t address); //done

    uint32_t get_occupancy (uint8_t queue_type, uint64_t address); //done
    uint32_t get_size (uint8_t queue_type, uint64_t address); //done

    void printout(void);

    // void schedule (PACKET_QUEUE *queue);
    // void process (PACKET_QUEUE *queue);
    // void update_schedule_cycle (PACKET_QUEUE *queue);
    // void update_process_cycle (PACKET_QUEUE *queue);
    // void reset_remain_requests (PACKET_QUEUE *queue, uint32_t channel);

    // Are the following required here ?
    // uint32_t dram_get_channel (uint64_t address);
    // uint32_t dram_get_rank (uint64_t address);
    // uint32_t dram_get_bank (uint64_t address);
    // uint32_t dram_get_row (uint64_t address);
    // uint32_t dram_get_column (uint64_t address);
    // uint32_t drc_check_hit (uint64_t address, uint32_t cpu, uint32_t channel,
    //     uint32_t rank, uint32_t bank, uint32_t row);

    // uint64_t get_bank_earliest_cycle (void);

    // int check_dram_queue (PACKET_QUEUE *queue, PACKET *packet);

};

extern NVDIMM VANS;
