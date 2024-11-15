#include <lvgl.h>
#include <TFT_eSPI.h>

#include <demos/lv_demos.h>
#include "touchpad.h"
#include "ui/ui.h"
TFT_eSPI tft = TFT_eSPI(); /* TFT实例 */

/*LVGL: Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  // bool touched = tft.getTouch( &touchX, &touchY, 600 );
  GT911_Scan();
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    /*Set the coordinates*/
    data->point.x = Dev_Now.X[0];
    data->point.y = Dev_Now.Y[0];
    // Serial.printf("touch:%d, x_in:%d, y_in:%d, x_out:%d, y_out:%d\r\n", touched, Dev_Now.X[0], Dev_Now.Y[0], data->point.x, data->point.y);
    data->state = LV_INDEV_STATE_PR;
  }
}

//////////////////////////// DISPLAY ///////////////////////////////
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 6];
// static lv_color_t buf[ screenWidth * 10 ];

/*LVGL: flush to display*/
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}
void setup()
{

  //  int y;
  Serial.begin(115200);

  //  y = touch.Y();
  String LVGL_Arduino = "Hello Arduino!9999";
  Serial.println(LVGL_Arduino);
  delay(100);
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println("I am LVGL_Arduino");
  GT911_Init_Touchpad();
  lv_init();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, 320, 480, TFT_RED);
  delay(500);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 6);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // /* 创建简单标签 */
  // //    lv_obj_t *label = lv_label_create( lv_scr_act() );
  // //    lv_label_set_text( label, LVGL_Arduino.c_str() );
  // //    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
  // lv_example_btn();
  ui_init();

  Serial.println("Setup done");
}

void loop()
{
  lv_timer_handler(); /* 让GUI完成它的工作 */
  ui_tick();
  delay(10);
}