#include <stdio.h>

#include "computation.h"
#include "event_queue.h"
#include "utils.h"

comp_t comp = {

    // TODO change the fractol structor
    .c_re = -0.4 * 1,
    .c_im = 0.6 * 1,
    // TODO change the color of the fractol
    .n = 60 * 1,

    // TODO zooming to center replace the 1 with a zoom factor >1 smaller <1 bigger
    .range_re_min = -1.6 * 1,
    .range_re_max = 1.6 * 1,
    .range_im_min = -1.1 * 1,
    .range_im_max = 1.1 * 1,

    .grid_w = 640,
    .grid_h = 480,

    .cur_x = 0,
    .cur_y = 0,

    .d_re = 0.0,
    .d_im = 0.0,

    .nbr_chunks = 0,
    .cid = 0,
    .chunk_re = 0.0,
    .chunk_im = 0.0,

    .chunk_n_re = 64,
    .chunk_n_im = 48,

    .grid = NULL,
    .computing = false,
    .abort = false,
    .done = false};

void computation_init(void) {
    comp.grid = my_alloc(comp.grid_w * comp.grid_h * sizeof(uint8_t));
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1. * comp.grid_w);
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1. * comp.grid_h);
    comp.nbr_chunks = (comp.grid_h / comp.chunk_n_im) * (comp.grid_w / comp.chunk_n_re);
}

void computation_cleanup(void) {
    if (comp.grid) {
        free(comp.grid);
    }
    comp.grid = NULL;
}

bool is_computing(void) { return comp.computing; }

bool is_done(void) { return comp.done; }

bool is_abort(void) { return comp.abort; }

void get_grid_size(int *w, int *h) {
    *w = comp.grid_w;
    *h = comp.grid_h;
}

void abort_comp(void) { comp.abort = true; }

void enable_comp(void) { comp.abort = false; }

bool set_compute(message *msg) {
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    bool ret = !is_computing();
    if (ret) {
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.n;
        comp.done = false;
    }
    return ret;
}

bool compute(message *msg) {
    if (!is_computing()) {  // first chunk
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0;        // start computation of the whole image
        comp.chunk_re = comp.range_re_min;  // left corner
        comp.chunk_im = comp.range_im_max;  // upper left corner
        msg->type = MSG_COMPUTE;
    } else {  // next chunk
        comp.cid += 1;
        if (comp.cid < comp.nbr_chunks) {
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w) {
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            }
            msg->type = MSG_COMPUTE;
        } else {  // all chunks computed
            // comp.computing = false;
            // comp.done = true;
            // msg->type = MSG_DONE;
        }
    }

    if (comp.computing && msg->type == MSG_COMPUTE) {
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing();
}

void update_image(int w, int h, unsigned char *img) {
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
    for (int i = 0; i < w * h; i++) {
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);
        *(img++) = 9 * (1 - t) * t * t * t * 255;
        *(img++) = 15 * (1 - t) * (1 - t) * t * t * 255;
        *(img++) = 8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255;
    }
}

void update_data(const msg_compute_data *compute_data) {
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);
    if (compute_data->cid == comp.cid) {
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if (idx >= 0 && idx < comp.grid_w * comp.grid_h) {
            comp.grid[idx] = compute_data->iter;
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re &&
            (compute_data->i_im + 1) == comp.chunk_n_im) {
            comp.done = true;
            comp.computing = false;
        }
    } else {
        warn("received chunk with unexpected chunk id (cid)\n");
    }
}

void cid_reset(void) {
    comp.abort = true;
    comp.done = true;
    comp.computing = false;
    comp.cid = 0;
}

void clear_buffer(void) {
    for (size_t i = 0; i < comp.grid_h * comp.grid_w * sizeof(uint8_t); i++)
    {
        comp.grid[i] = 0;
    }
}

void cpu_comp(void) {
    for (size_t i = 0; i < comp.grid_h; i++)
    {
        for (size_t j = 0; j < comp.grid_w; j++)
        {
            comp.grid[i * comp.grid_w + j] = cpu_fractal(j, i);
        }
    }
}

int cpu_fractal(int x, int y) {
    y = comp.grid_h - y;
    double scale_x = (comp.range_re_max - comp.range_re_min);
    double scale_y = (comp.range_im_max - comp.range_im_min);
    double mapped_x = ((double)x / (double)comp.grid_w) * scale_x - scale_x / 2;
    double mapped_y = ((double)y / (double)comp.grid_h) * scale_y - scale_y / 2;
    double complex z = mapped_x + mapped_y * I;
	double complex c = comp.c_re + comp.c_im * I;
	//init calc
	z = complex_pow(z) + c;
    int iteration = 1;
    
    while (complex_abs(z) < 2 && iteration <= comp.n) {
        z = complex_pow(z) + c;
        if (iteration != comp.n)
		{
			iteration++;
		}
    }
    return iteration;
}

bool set_module_compute(message *msg) {
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    bool ret = !is_computing();
    if (ret) {
        comp.c_re = msg->data.set_compute.c_re;
        comp.c_im = msg->data.set_compute.c_im;
        comp.d_re = msg->data.set_compute.d_re;
        comp.d_im = msg->data.set_compute.d_im;
        comp.n = msg->data.set_compute.n;
    }
    return ret;
}

void module_compute(message *msg) {
    comp.cid = msg->data.compute.cid;
    comp.chunk_re = msg->data.compute.re;
    comp.chunk_im = msg->data.compute.im;
    comp.chunk_n_re = msg->data.compute.n_re;
    comp.chunk_n_im = msg->data.compute.n_im;

    for (u_int8_t i = 0; i < comp.chunk_n_re; i++)
    {
        for (u_int8_t j = 0; j < comp.chunk_n_im; j++)
        {
            double complex z = i + j * I;
            double complex c = comp.c_re + comp.c_im * I;
            //init calc
            z = complex_pow(z) + c;
            u_int8_t iteration = 1;
            
            while (complex_abs(z) < 2 && iteration <= comp.n) {
                z = complex_pow(z) + c;
                if (iteration != comp.n)
                {
                    iteration++;
                }
            }
            event ev = { .type = EV_COMPUTE_DATA};
            ev.data.msg->data.compute_data.cid = comp.cid;
            ev.data.msg->data.compute_data.i_re = i;
            ev.data.msg->data.compute_data.i_im = j;
            ev.data.msg->data.compute_data.iter = iteration;
            queue_push(ev);
        }
    }
}

double complex complex_pow(double complex z) {
    double real = creal(z);
    double imag = cimag(z);
    double realPart = real * real - imag * imag;
    double imagPart = real * imag * 2;
    return realPart + imagPart * I;
}

double complex_abs(double complex z) {
    double real = creal(z);
    double imag = cimag(z);
    return (my_sqrt(real * real + imag * imag));
}

double my_sqrt(double x) {
    //special case for 0 and 1
    if (x < 2){
        return x;
    }
    double y = x;
    double z = (y + (x / y)) / 2;
    //continue iterating until the difference between y and z is less than 0.00001
    while (my_abs(y - z) >= 0.00001) {
        y = z;
        z = (y + (x / y)) / 2;
    }
    return z;
}

double my_abs(double x) {
    if (x < 0)
    {
        return x * -1;
    }
    return x;
}

/* end of computation.c */
