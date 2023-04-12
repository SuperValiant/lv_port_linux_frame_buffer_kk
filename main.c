#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drv_conf.h"
#if USE_SDL
#include "lv_drivers/sdl/sdl.h"
#else
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#endif
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define DISP_BUF_SIZE (128 * 1024)

static int g_val = 1;
static lv_obj_t *cen_label = NULL;

static void up_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_LONG_PRESSED_REPEAT || code == LV_EVENT_PRESSED)
    {
        g_val++;
        if (g_val >= 100)
        {
            g_val = 0;
        }
        LV_LOG_USER("UP\n");
        lv_label_set_text_fmt(cen_label, "Val: %02d", g_val);
    }
}

static void dw_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_LONG_PRESSED_REPEAT || code == LV_EVENT_PRESSED)
    {
        g_val--;
        if (g_val < 0)
        {
            g_val = 99;
        }
        LV_LOG_USER("DOWN\n");
        lv_label_set_text_fmt(cen_label, "Val: %02d", g_val);
    }
}

static void my_button_and_label(void)
{
    lv_obj_t *parent = lv_obj_create(lv_scr_act());

    lv_obj_set_width(parent, 320);
    lv_obj_set_height(parent, 240);
    lv_obj_center(parent);
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x94eaff), 0);

    lv_obj_t *up_button = lv_btn_create(parent);
    lv_obj_t *dw_button = lv_btn_create(parent);
    cen_label = lv_label_create(parent);

    lv_obj_set_width(up_button, 80);
    lv_obj_set_height(up_button, 50);
    lv_obj_set_width(dw_button, 80);
    lv_obj_set_height(dw_button, 50);
    lv_obj_set_width(cen_label, 150);
    lv_obj_set_height(cen_label, 50);
    lv_obj_align(up_button, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align(dw_button, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_align(cen_label, LV_ALIGN_BOTTOM_MID, 0, -65);
    lv_obj_set_style_text_font(cen_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_align(cen_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    //lv_obj_set_style_bg_color(cen_label, lv_color_hex(0x00ff00), 0);
    //lv_obj_set_style_bg_opa(cen_label, 70, 0);

    lv_obj_add_event_cb(up_button, up_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(dw_button, dw_event_handler, LV_EVENT_ALL, NULL);

    lv_label_set_text_fmt(cen_label, "Val: %02d", g_val);

    lv_obj_t *up_label = lv_label_create(up_button);
    lv_obj_t *dw_label = lv_label_create(dw_button);

    lv_obj_center(up_label);
    lv_obj_center(dw_label);
    lv_label_set_text(up_label, "ADD");
    lv_label_set_text(dw_label, "MINUS");
}

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
#if USE_SDL
    sdl_init();
#else
    fbdev_init();
#endif

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
#if USE_SDL
    disp_drv.flush_cb   = sdl_display_flush;
    disp_drv.hor_res    = SDL_HOR_RES;
    disp_drv.ver_res    = SDL_VER_RES;
#else	//Example x2100
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 320;
    disp_drv.ver_res    = 240;
    disp_drv.sw_rotate = 1;
    disp_drv.rotated = LV_DISP_ROT_180;
#endif
    lv_disp_drv_register(&disp_drv);

#if USE_SDL
    //evdev_init();
#else
    evdev_init();
#endif
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
#if USE_SDL
    indev_drv_1.read_cb = sdl_mouse_read;
#else
    indev_drv_1.read_cb = evdev_read;
#endif
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);


    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/


    /*Create a Demo*/
    //lv_demo_widgets();

    //addkk button demo
    my_button_and_label();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
