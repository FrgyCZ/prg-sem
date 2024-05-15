#include <stdbool.h>
#include <complex.h>

#include "messages.h"

#ifndef __COMPUTATION_H__
#define __COMPUTATION_H__

#define ZOOM_COEFFICIENT 2

typedef struct {
    double c_re;
    double c_im;
    double n;

    double range_re_min;
    double range_re_max;
    double range_im_min;
    double range_im_max;

    int grid_w;
    int grid_h;

    int cur_x;
    int cur_y;

    double d_re;
    double d_im;

    int nbr_chunks;
    int cid;
    double chunk_re;
    double chunk_im;

    uint8_t chunk_n_re;
    uint8_t chunk_n_im;

    uint8_t *grid;
    bool computing;
    bool abort;
    bool done;

    double zoom_level;
} comp_t;

void computation_init(void);
void computation_cleanup(void);

void get_grid_size(int *w, int *h);
bool is_computing(void);
bool is_done(void);
bool is_abort(void);

void abort_comp(void);
void enable_comp(void);

bool set_compute(message *msg);
bool compute(message *msg);

void update_image(int w, int h, unsigned char *img);
void update_data(const msg_compute_data *compute_data);

void cid_reset(void);
void clear_buffer(void);

void cpu_comp(void);
int cpu_fractal(int x, int y);

double complex complex_pow(double complex z);
double complex_abs(double complex z);
double my_sqrt(double x);
double my_abs(double x);

void zoom(double scale);
void move(double x, double y);

void change_c_re(double x);
void change_c_im(double y);

void get_info(char *c, char *depth);

#endif

/* end of computation.h */
