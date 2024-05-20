/*
 * Filename: xwin_sdl.h
 * Date:     2015/06/18 14:37
 * Author:   Jan Faigl
 */

#include <stdbool.h>

#ifndef __XWIN_SDL_H__
#define __XWIN_SDL_H__

int xwin_init(int w, int h);
void xwin_close();
void xwin_display_startup_message(void);
void xwin_redraw(int w, int h, unsigned char *img, const char *c_text, const char *depth_text);
void change_window_size(int w, int h);
void render_text_to_win(const char *text, int x, int y, bool info_text);
void xwin_poll_events(void);

void save_image(void);
void record_animation(int frame);

#endif

/* end of xwin_sdl.h */
