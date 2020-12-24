#include "imc.h"

namespace vans::imc
{

// base_request_queue imc_controller::get_rpq()
// {
//     return rpq;
// }
//
// base_request_queue imc_controller::get_wpq()
// {
//     return wpq;
// }

// base_response imc_controller::issue_request(base_request &request)
// {
//     bool success = false;
//     switch (request.type) {
//     case base_request_type::read:
//         success = rpq.enqueue(request);
//         break;
//     case base_request_type::write:
//         success = wpq.enqueue(request);
//         break;
//     }
//
//     return {(success), false, clk_invalid};
// }

base_response imc_controller::issue_request(base_request &request)
{
    bool success = false;
    
    if (request.operation == 0)
    {
        // issue the request
        if (request.type == base_request_type::read)
        {
            // check for latest writebacks in write queue
            for (size_t i = 0; i < wpq.size(); i++)
            {
                if (wpq.queue[i].addr == request.addr)
                    return { false, false, clk_invalid, -2};
            }

            // check for duplicates in read queue
            for (size_t i = 0; i < rpq.size(); i++)
            {
                if (rpq.queue[i].addr == request.addr)
                    return { false, false, clk_invalid, i};
            }

            success = rpq.enqueue(request);
        }
        else if (request.type == base_request_type::write)
        {
            // check for duplicates in write queue
            for (size_t i = 0; i < wpq.size(); i++)
            {
                if (wpq.queue[i].addr == request.addr)
                    return { false, false, clk_invalid, i};
            }

            success = wpq.enqueue(request);
        }
    }
    else if (request.operation == 1)
    {
        // get size
        if (request.type == base_request_type::read)
            return { false, false, clk_invalid, (int)rpq.max_entries};
        else if (request.type == base_request_type::write)
            return { false, false, clk_invalid, (int)wpq.max_entries};
    }
    else if (request.operation == 2)
    {
        // get occupancy
        if (request.type == base_request_type::read)
            return { false, false, clk_invalid, (int)rpq.size()};
        else if (request.type == base_request_type::write)
            return { false, false, clk_invalid, (int)wpq.size()};
    }

    return {(success), false, clk_invalid, -1};
}

void imc_controller::tick(clk_t curr_clk)
{
    this->imc_curr_clk = curr_clk;

    /* Tick wpq and rpq in imc:
     *   issues write request from wpq
     *   issues read request from rpq
     */
    enum { none = 0, read, write } queue_to_tick;

    bool rpq_empty = rpq.empty();
    bool wpq_empty = wpq.empty();

    if (rpq_empty && wpq_empty) {
        queue_to_tick = none;
    } else if (rpq_empty) {
        queue_to_tick = write;
    } else if (wpq_empty) {
        queue_to_tick = read;
    } else {
        /* First come first serve */
        if (rpq.queue.front().arrive < wpq.queue.front().arrive) {
            queue_to_tick = read;
        } else {
            queue_to_tick = write;
        }
    }

    if (queue_to_tick == read) {
        auto &req = rpq.queue.front();
        auto next = this->get_next_level(req.addr);
        if (!next->full()) {
            auto [issued, deterministic, next_clk, extraval] = next->issue_request(req);
            if (issued) {
                rpq.queue.pop_front();
            }
        }
    } else if (queue_to_tick == write) {
        if (wpq.full()) {
            flush_wpq();
        }
    } else {
        /* No queue to tick */
    }

    if (!wpq.empty()) {
        adr();
    }
}

void imc_controller::adr()
{
    if (this->adr_epoch != 0) {
        if ((imc_curr_clk + 1) % this->adr_epoch == 0) {
            flush_wpq();
        }
    }
}

void imc_controller::flush_wpq()
{
    while (!wpq.empty()) {
        auto &req = wpq.queue.front();

        if (!rpq.empty()) {
            if (rpq.queue.front().arrive < req.arrive) {
                break;
            }
        }

        auto next = this->get_next_level(req.addr);
        if (next->full()) {
            break;
        }

        auto [issued, deterministic, next_clk, extraval] = next->issue_request(req);
        if (issued) {
            if (req.callback) {
                req.callback(req.addr, imc_curr_clk);
            }
            wpq.queue.pop_front();
        }
    }
}

} // namespace vans::imc
