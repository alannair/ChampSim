#include <iostream>
#include <string>
#include <unistd.h>
// #include "memory_class.h"
#include "config.h"
#include "factory.h"
#include "trace.h"

using namespace std;

class NVDIMM
{
public:
    string config_filename;
    std::shared_ptr<vans::base_component> model;

    // NVDIMM()
    // {};

    NVDIMM(string filename) : config_filename(filename)
    {
        auto cfg   = vans::root_config(config_filename);
        auto model = vans::factory::make(cfg);
    };

    ~NVDIMM()
    {};

    //methods
    // int add_rq (PACKET )
    void init(void);
};

extern NVDIMM VANS;
