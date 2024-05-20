#ifndef __GUI_H__
#define __GUI_H__

void gui_init(void);
void gui_cleanup(void);
void gui_refresh(void);
void render_text(void);
void gui_save_image(void);
void gui_record_animation(int frame);
void gui_change_window_size(int w, int h);
void display_startup_message(void);
void gui_run_animation(void);

void *gui_win_thread(void *d);

#endif

/* end of gui.h */
