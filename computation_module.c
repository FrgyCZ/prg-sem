#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <complex.h>

#include "computation.h"
#include "computation_module.h"
#include "event_queue.h"
#include "messages.h"
#include "prg_io_nonblock.h"
#include "utils.h"
#include <sys/types.h>

comp_module_t comp_module = {
    .c_re = 0,
    .c_im = 0,
    .n = 0,

    .d_re = 0,
    .d_im = 0,

    .cid = 0,
    .chunk_re = 0,
    .chunk_im = 0,

    .chunk_n_re = 0,
    .chunk_n_im = 0,

    .computing = false,
    .abort = false,
    .done = false,

    .pipe_out = -1
};

bool set_module_compute(message *msg, void *d) {
    comp_module.c_re = msg->data.set_compute.c_re;
    comp_module.c_im = msg->data.set_compute.c_im;
    comp_module.d_re = msg->data.set_compute.d_re;
    comp_module.d_im = msg->data.set_compute.d_im;
    comp_module.n = msg->data.set_compute.n;
    comp_module.pipe_out = *(int *)d;
    return true;
}

void module_compute(message *msg_incoming) {
    if (comp_module.abort){
        message msg_to_send;
        uint8_t msg_buf[sizeof(message)];
        int msg_len;
        msg_to_send.type = MSG_ABORT;
        my_assert(fill_message_buf(&msg_to_send, msg_buf, sizeof(msg_buf), &msg_len), __func__, __LINE__, __FILE__);
        if (!(write(comp_module.pipe_out, msg_buf, msg_len) == msg_len)) {
            error("send message fail\n");
        }
    }
    else {
        comp_module.cid = msg_incoming->data.compute.cid;
        comp_module.chunk_re = msg_incoming->data.compute.re;
        comp_module.chunk_im = msg_incoming->data.compute.im;
        comp_module.chunk_n_re = msg_incoming->data.compute.n_re;
        comp_module.chunk_n_im = msg_incoming->data.compute.n_im;

        for (u_int8_t i = 0; i < comp_module.chunk_n_re; i++)
        {
            for (u_int8_t j = 0; j < comp_module.chunk_n_im; j++)
            {
                double complex z = (i * comp_module.d_re + comp_module.chunk_re) + (j * comp_module.d_im + comp_module.chunk_im) * I;
                double complex c = comp_module.c_re + comp_module.c_im * I;
                //init calc
                z = complex_pow(z) + c;
                u_int8_t iteration = 1;
                
                while (complex_abs(z) < 2 && iteration <= comp_module.n) {
                    z = complex_pow(z) + c;
                    if (iteration != comp_module.n)
                    {
                        iteration++;
                    }
                }
                message msg_to_send;
                uint8_t msg_buf[sizeof(message)];
                int msg_len;
                msg_to_send.type = MSG_COMPUTE_DATA;
                msg_to_send.data.compute_data.cid = comp_module.cid;
                msg_to_send.data.compute_data.i_re = i;
                msg_to_send.data.compute_data.i_im = j;
                msg_to_send.data.compute_data.iter = iteration;
                //printf("cid: %d, re: %d, im: %d, iter: %d\n", msg_to_send.data.compute_data.cid, msg_to_send.data.compute_data.i_re, msg_to_send.data.compute_data.i_im, msg_to_send.data.compute_data.iter);
                my_assert(fill_message_buf(&msg_to_send, msg_buf, sizeof(msg_buf), &msg_len), __func__, __LINE__, __FILE__);
                if (!(write(comp_module.pipe_out, msg_buf, msg_len) == msg_len)) {
                    error("send message fail\n");
                }
            }
        }
        event ev = { .type = EV_COMPUTE_DATA_DONE};
        queue_push(ev);
    }
}

void abort_module_compute(void) {
    comp_module.abort = true;
}

void enable_module_compute(void) {
    comp_module.abort = false;
}

bool is_module_aborted(void) {
    return comp_module.abort;
}