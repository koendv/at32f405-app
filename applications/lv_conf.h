#ifndef __LV_CONF_H__
#define __LV_CONF_H__

#define LV_CONF_SUPPRESS_DEFINE_CHECK 1
#define LV_USE_ST7789                 1
#define LV_COLOR_DEPTH                16
#define LV_COLOR_16_SWAP              0
#define LV_DPI_DEF                    216
#define LV_USE_OS                     LV_OS_RTTHREAD
#define LV_USE_LOG                    1
#define LV_LOG_LEVEL                  LV_LOG_LEVEL_INFO
#define LV_USE_ASSERT_NULL            1 /*Check if the parameter is NULL. (Very fast, recommended)*/
#define LV_USE_ASSERT_MALLOC          1 /*Checks is the memory is successfully allocated or no. (Very fast, recommended)*/
#define LV_USE_ASSERT_STYLE           1 /*Check if the styles are properly initialized. (Very fast, recommended)*/
#define LV_USE_ASSERT_MEM_INTEGRITY   0 /*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#define LV_USE_ASSERT_OBJ             0 /*Check the object's type and existence (e.g. not deleted). (Slow)*/
#define LV_USE_TILEVIEW               1
#define LV_FONT_MONTSERRAT_12         1
#define LV_FONT_MONTSERRAT_16         1
#define LV_FONT_MONTSERRAT_22         1
#define LV_FONT_MONTSERRAT_28         1
#define LV_FONT_DEFAULT               &lv_font_montserrat_22

/* lvgl mutex */
extern void lv_lock(void);
extern void lv_unlock(void);

#endif
