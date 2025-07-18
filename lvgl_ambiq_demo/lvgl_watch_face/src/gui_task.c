//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "lvgl.h"
#include "lv_draw_ambiq_private.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "gui_task.h"
#include "lvgl_test.h"
#include "tsi_malloc.h"
#include "nema_utils.h"
// #include "lv_img_buf.h"
#include "texture/watch_bg.h"
#include "texture/hour.h"
#include "texture/minute.h"
#include "texture/second.h"

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
SemaphoreHandle_t lvgl_mutex = NULL;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

void lv_ambiq_log_printf(lv_log_level_t level, const char * buf)
{
    g_pfnCharPrint((char *)buf);
}

void lv_example_loading_font(void)
{
    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t * label2 = lv_label_create(lv_screen_active());

    char az_text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    lv_label_set_text(label2, az_text);
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_width(label2, 300);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_26, 0);

    char az_text_1[] = "abcdefghijklmnopqrstuvwxyz0123456789";

    lv_obj_t * label3 = lv_label_create(lv_screen_active());
    lv_label_set_text(label3, az_text_1);
    lv_obj_set_width(label3, 300);
    lv_label_set_long_mode(label3, LV_LABEL_LONG_WRAP);
    lv_font_t *my_font = lv_binfont_create("A:/ubuntu.bin");
    if (my_font == NULL)
    {
        LV_LOG_ERROR("Failed to load font from A:/ubuntu.bin");
        return;
    }
    lv_obj_set_style_text_font(label3, my_font, 0);
    lv_obj_align(label3, LV_ALIGN_TOP_MID, 0, 150);
}

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    int ret;

    am_util_stdio_printf("Gui task start!\n");
    //
    // Init file system
    //
    LV_LOG_INFO("Init file system...\r\n");
    lv_ambiq_fs_init();
    //
    // Init LVGL.
    //
    lv_init();

    lv_tick_set_cb(xTaskGetTickCount);

#if LV_USE_LOG == 1
    lv_log_register_print_cb(lv_ambiq_log_printf);
#endif

    //
    // Iint display
    //
    LV_LOG_INFO("setup display...\r\n");
    lv_ambiq_display_init();

    //
    // Set up LVGL touch driver: init touch device and set it as the input device for lvgl.
    //
    LV_LOG_INFO("setup touch...\r\n");
    lv_ambiq_touch_init();

    /* Create a mutex to avoid the concurrent calling of LVGL functions. */
    lvgl_mutex = xSemaphoreCreateMutex();
    if( lvgl_mutex == NULL )
    {
        LV_LOG_ERROR("LVGL mutex create failed!\n");
        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    /*Now create the actual image*/
    lv_obj_t *obj_watch_bg = lv_img_create(lv_scr_act());
    lv_img_set_src(obj_watch_bg, &watch_bg);
    lv_obj_align(obj_watch_bg, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *lv_hour = lv_img_create(obj_watch_bg);
    lv_img_set_src(lv_hour, &hour);
    lv_obj_align(lv_hour, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_hour, 25, 224);

    lv_obj_t *lv_minute = lv_img_create(obj_watch_bg);
    lv_img_set_src(lv_minute, &minute);
    lv_obj_align(lv_minute, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_minute, 25, 224);

    lv_obj_t *lv_second = lv_img_create(obj_watch_bg);
    lv_img_set_src(lv_second, &second);
    lv_obj_align(lv_second, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_second, 25, 224);

    // lv_obj_t * lv_dot_l = lv_img_create(watch_bg);
    // lv_img_set_src(lv_dot_l, &img_dot_l_psram);
    // lv_obj_align(lv_dot_l, LV_ALIGN_CENTER, 0, 0);

    // lv_obj_t * lv_dot_m = lv_img_create(watch_bg);
    // lv_img_set_src(lv_dot_m, &img_dot_m_psram);
    // lv_obj_align(lv_dot_m, LV_ALIGN_CENTER, 0, 0);

    // lv_obj_t * lv_dot_s = lv_img_create(watch_bg);
    // lv_img_set_src(lv_dot_s, &img_dot_s_psram);
    // lv_obj_align(lv_dot_s, LV_ALIGN_CENTER, 0, 0);

    // lv_obj_t * lv_dot_t = lv_img_create(watch_bg);
    // lv_img_set_src(lv_dot_t, &img_dot_t_psram);
    // lv_obj_align(lv_dot_t, LV_ALIGN_CENTER, 0, 0);

    uint32_t last_sec = 0;
    uint32_t current_sec = 0;
    bool watch_update = false;
    uint32_t needle_hour = 0;
    uint32_t needle_minute = 0;
    uint32_t needle_second = 0;
    uint32_t angle;

    while(1)
    {
        // current_sec = (uint32_t) nema_get_time();
        current_sec += 1;

        if (current_sec != last_sec)
        {
            last_sec = current_sec;
            watch_update = true;

            if (++needle_second > 59)
            {
                needle_second = 0;
                needle_minute++;

                if (needle_minute > 59)
                {
                    needle_minute = 0;
                    needle_hour++;
                    if (needle_hour > 11)
                    {
                        needle_hour = 0;
                    }
                }
            }
        }

        if (watch_update == true)
        {
            angle = needle_hour * 300 + needle_minute * 5;
            lv_img_set_angle(lv_hour, angle);

            angle = needle_minute * 60 + needle_second;
            lv_img_set_angle(lv_minute, angle);

            angle = needle_second * 60;
            lv_img_set_angle(lv_second, angle);

            watch_update = false;
        }

        uint32_t time_till_next;

        // Run timer handler.
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
        time_till_next = lv_timer_handler();
        xSemaphoreGive(lvgl_mutex);

        // Delay
        vTaskDelay(time_till_next);

    }
}