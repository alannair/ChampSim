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
};

class NVDIMM : public MEMORY
{
public:
    string config_filename;
    std::shared_ptr<vans::base_component> model;
    int fill_level;
    int cpu;
    vector<struct PENDING_REQUESTS> outstanding;
    int count;
    int TOTAL_ACCESSES, READ_ACCESSES, WRITE_ACCESSES, LSQ_STALLS, LSQ_FULL_CYCLES;
    bool lsq_stall;

    NVDIMM(string filename, int fill) :
     config_filename(filename), fill_level(fill)
    {
        cpu = 0; // a single-core machine
        count = 0;
        auto cfg   = vans::root_config(config_filename);
        model = vans::factory::make(cfg);

        TOTAL_ACCESSES = READ_ACCESSES = WRITE_ACCESSES = LSQ_STALLS = LSQ_FULL_CYCLES = 0;
        lsq_stall = false;

        cout << "DDR4 SIZE: " << cfg.at("ait").cfg["size"] << "MB\t Channels: "
            << cfg.at("ait").cfg["channel"] << "\t Width: "
            << 8*stoi(cfg.at("ait").cfg["data_width"]) << "-bit\t Data Rate: "
            << cfg.at("ait").cfg["rate"] << " MT/S" << endl;
    };

    ~NVDIMM()
    {};

    // methods
    void init (void);

    int add_rq (PACKET *packet);
    int add_wq (PACKET *packet);
    int add_pq (PACKET *packet);

    void return_data (PACKET *packet);
    void operate (void);
    void increment_WQ_FULL (uint64_t address);

    uint32_t get_occupancy (uint8_t queue_type, uint64_t address);
    uint32_t get_size (uint8_t queue_type, uint64_t address);

    void printout(void);
    void drain(void);
    void print_stats(void);
    void reset_stats(void);
};

extern NVDIMM VANS;
