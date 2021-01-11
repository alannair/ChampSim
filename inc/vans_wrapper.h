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
    vector<struct PENDING_REQUESTS> outstanding;
    float t_CK;
    uint64_t TOTAL_LATENCY[NUM_CPUS+1], LOAD_LATENCY[NUM_CPUS+1],  WRITE_LATENCY[NUM_CPUS+1], PREFETCH_LATENCY[NUM_CPUS+1], RFO_LATENCY[NUM_CPUS+1];
    uint32_t TOTALS[NUM_CPUS+1], LOADS[NUM_CPUS+1], WRITES[NUM_CPUS+1], PREFETCHES[NUM_CPUS+1], RFOS[NUM_CPUS+1];

    NVDIMM(string filename, int fill) :
     config_filename(filename), fill_level(fill)
    {
        auto cfg   = vans::root_config(config_filename);
        model = vans::factory::make(cfg);
        t_CK = stof(cfg.at("ait").cfg["tCK"]);

        for (int i=0; i<=NUM_CPUS; ++i)
            TOTAL_LATENCY[i] = LOAD_LATENCY[i] = WRITE_LATENCY[i] = PREFETCH_LATENCY[i] = RFO_LATENCY[i] = TOTALS[i] = LOADS[i] = WRITES[i] = PREFETCHES[i] = RFOS[i] = 0;


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
