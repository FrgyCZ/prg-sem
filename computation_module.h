#include <stdbool.h>
#include <complex.h>

#include "event_queue.h"
#include "messages.h"

#ifndef __COMPUTATION_MODULE_H__
#define __COMPUTATION_MODULE_H__

typedef struct {
    double c_re;
    double c_im;
    int n;

    double d_re;
    double d_im;

    int cid;
    double chunk_re;
    double chunk_im;

    uint8_t chunk_n_re;
    uint8_t chunk_n_im;

    bool computing;
    bool abort;
    bool done;

    int pipe_out;
} comp_module_t;

bool set_module_compute(message *msg, void *d);
void module_compute(message *msg_incoming);

void abort_module_compute(void);
void enable_module_compute(void);

bool is_module_aborted(void);

#endif
