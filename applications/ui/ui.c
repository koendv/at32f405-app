#include "lvgl.h"
#include "app.h"

#define TILE_NUM 3

lv_obj_t *tv             = NULL;
lv_obj_t *tile[TILE_NUM] = {NULL};
lv_obj_t *target_txt     = NULL;
lv_obj_t *icons_txt      = NULL;
lv_obj_t *progress_bar   = NULL;
uint32_t  cnt            = 1;
uint32_t  current_tile   = 0;

void set_target_text(const char *txt)
{
    if (target_txt && txt)
        lv_label_set_text(target_txt, txt);
}

static void scr_gesture_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    switch (dir)
    {
    case LV_DIR_TOP:
        lv_label_set_text(target_txt, "swipe down");
        break;
    case LV_DIR_BOTTOM:
        lv_label_set_text(target_txt, "swipe up");
        break;
    case LV_DIR_LEFT:
        lv_label_set_text(target_txt, "swipe left");
        current_tile = (current_tile + 1) % TILE_NUM;
        lv_tileview_set_tile_by_index(tv, current_tile, 0, LV_ANIM_OFF);
        break;

    case LV_DIR_RIGHT:
        lv_label_set_text(target_txt, "swipe right");
        current_tile = (current_tile + TILE_NUM - 1) % TILE_NUM;
        lv_tileview_set_tile_by_index(tv, current_tile, 0, LV_ANIM_OFF);
        break;
    default:
        lv_label_set_text(target_txt, "?");
        break;
    }
    cnt = (cnt + 1) % 100;
    lv_obj_update_flag(progress_bar, LV_OBJ_FLAG_HIDDEN, false);
    lv_bar_set_value(progress_bar, cnt, LV_ANIM_OFF);
}

static void scr_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_LONG_PRESSED:
        lv_label_set_text(target_txt, "long press");
        break;
    case LV_EVENT_CLICKED:
        lv_label_set_text(target_txt, "click");
        break;
    case LV_EVENT_DOUBLE_CLICKED:
        lv_label_set_text(target_txt, "double click");
        break;
    default:
        lv_label_set_text(target_txt, "?");
        break;
    }
    cnt = (cnt + 1) % 100;
    lv_obj_update_flag(progress_bar, LV_OBJ_FLAG_HIDDEN, false);
    lv_bar_set_value(progress_bar, cnt, LV_ANIM_OFF);
}

void ui_init()
{
    lv_obj_t *obj;

    lv_lock();

    /* set screen background to white */
    lv_obj_t *scr = lv_screen_active();
    LV_ASSERT_MALLOC(scr);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);

    /* capture gestures */
    lv_obj_add_event_cb(scr, scr_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr, scr_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_add_event_cb(scr, scr_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(scr, scr_event_cb, LV_EVENT_DOUBLE_CLICKED, NULL);

    /* tile view */
    tv = lv_tileview_create(lv_screen_active());
    for (uint32_t i = 0; i < TILE_NUM; i++)
        tile[i] = lv_tileview_add_tile(tv, i, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

    /* create progress bar */
    progress_bar = lv_bar_create(tile[0]);
    lv_obj_set_width(progress_bar, 200);
    lv_obj_center(progress_bar);
    lv_obj_update_flag(progress_bar, LV_OBJ_FLAG_HIDDEN, true);

    /* create target name label */
    target_txt = lv_label_create(tile[0]);
    lv_label_set_text(target_txt, "");
    lv_obj_align(target_txt, LV_ALIGN_TOP_MID, 0, 60);

    /* create icon label */
    icons_txt = lv_label_create(tile[0]);
    lv_label_set_text(icons_txt, "");
    lv_obj_align(icons_txt, LV_ALIGN_TOP_MID, 0, 0);

    if (sdcard_mounted) lv_label_set_text(icons_txt, LV_SYMBOL_SD_CARD);

    lv_obj_t *txt = lv_label_create(tile[1]);
    lv_label_set_text(txt, "tile 1");
    lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);

    txt = lv_label_create(tile[2]);
    lv_label_set_text(txt, "tile 2");
    lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);

    lv_unlock();
}
