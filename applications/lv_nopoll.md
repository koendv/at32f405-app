lv_nopoll.c is an lvgl input device that does not use polling.
Initialize with
```
lv_nopoll_create();
```
To use, just call
```
lv_nopoll_click(x, y);
```
when the user touches touchscreen coordinates (x, y).
That is all.

The following calls are available for touch controllers that do on-chip gesture decoding:
```
lv_nopoll_swipe_up(x, y);
lv_nopoll_swipe_down(x, y);
lv_nopoll_swipe_left(x, y);
lv_nopoll_swipe_right(x, y);
lv_nopoll_long_press(x, y);
```
The x and y coordinates are int32_t.
