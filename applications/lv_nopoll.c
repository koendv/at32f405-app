#include "lvgl.h"
#include "lv_indev.h"
#include "lv_indev_private.h"
#include "lv_nopoll.h"

static lv_indev_t     *_indev  = NULL;
static lv_obj_t       *_obj   = NULL;
static lv_event_code_t _event = LV_EVENT_ALL;
static lv_dir_t        _dir   = LV_DIR_NONE;

static void      indev_read(lv_indev_t *indev, lv_indev_data_t *data);
static lv_obj_t *pointer_search_obj(int32_t x, int32_t y);
static void      lv_nopoll_event(int32_t x, int32_t y, lv_event_code_t event);
static void      lv_nopoll_gesture(int32_t x, int32_t y, lv_dir_t dir);

void lv_nopoll_create()
{
    _indev = lv_indev_create();
    lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_mode(_indev, LV_INDEV_MODE_EVENT);
    lv_indev_set_read_cb(_indev, indev_read);
}

void lv_nopoll_click(int32_t x, int32_t y)
{
    lv_nopoll_event(x, y, LV_EVENT_CLICKED);
}

void lv_nopoll_long_press(int32_t x, int32_t y)
{
    lv_nopoll_event(x, y, LV_EVENT_LONG_PRESSED);
}

void lv_nopoll_double_click(int32_t x, int32_t y)
{
    lv_nopoll_event(x, y, LV_EVENT_DOUBLE_CLICKED);
}

void lv_nopoll_swipe_up(int32_t x, int32_t y)
{
    lv_nopoll_gesture(x, y, LV_DIR_TOP);
}

void lv_nopoll_swipe_down(int32_t x, int32_t y)
{
    lv_nopoll_gesture(x, y, LV_DIR_BOTTOM);
}

void lv_nopoll_swipe_left(int32_t x, int32_t y)
{
    lv_nopoll_gesture(x, y, LV_DIR_LEFT);
}

void lv_nopoll_swipe_right(int32_t x, int32_t y)
{
    lv_nopoll_gesture(x, y, LV_DIR_RIGHT);
}

/* return object at coordinates (x, y) */
static lv_obj_t *pointer_search_obj(int32_t x, int32_t y)
{
    lv_obj_t  *obj = NULL;
    lv_point_t p;

    p.x = x;
    p.y = y;

    /* code from lv_indev.c */
    obj = lv_indev_search_obj(lv_layer_sys(), &p);
    if (obj) return obj;

    obj = lv_indev_search_obj(lv_layer_top(), &p);
    if (obj) return obj;

    /* Search the object in the active screen */
    obj = lv_indev_search_obj(lv_screen_active(), &p);
    if (obj) return obj;

    obj = lv_indev_search_obj(lv_layer_bottom(), &p);
    return obj;
}

static void lv_nopoll_event(int32_t x, int32_t y, lv_event_code_t event)
{
    _obj   = pointer_search_obj(x, y);
    _event = event;
    _dir   = LV_DIR_NONE;

    /* send event to object. if object has event bubble flag set, send event to parent too */
    while (_obj)
    {
        lv_indev_read(_indev); // send event to _obj
        if (!lv_obj_has_flag(_obj, LV_OBJ_FLAG_EVENT_BUBBLE))
            break;
        _obj = lv_obj_get_parent(_obj);
    }
    lv_disp_trig_activity(NULL);
}

static void lv_nopoll_gesture(int32_t x, int32_t y, lv_dir_t dir)
{
    _obj   = pointer_search_obj(x, y);
    _event = LV_EVENT_GESTURE;
    _dir   = dir;

    /* send gesture to object. if object has gesture bubble flag set, send gesture to parent instead */
    while (_obj && lv_obj_has_flag(_obj, LV_OBJ_FLAG_GESTURE_BUBBLE))
        _obj = lv_obj_get_parent(_obj);
    if (_obj) lv_indev_read(_indev); // send gesture to _obj
    lv_disp_trig_activity(NULL);
}

/* called by lvgl to read input device */
static void indev_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (indev) indev->pointer.gesture_dir = _dir;
    lv_obj_send_event(_obj, _event, indev);
    if (data) data->state = LV_INDEV_STATE_RELEASED;
}
