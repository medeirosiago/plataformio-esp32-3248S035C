#include <lvgl.h>
#include <TFT_eSPI.h>

#include <demos/lv_demos.h>
#include "utils/home-assistant/HomeAssistant.h"

#include "touchpad.h"
#include "ui/ui.h"

TFT_eSPI tft = TFT_eSPI();
const char *ssid = "Tatakae";
const char *password = "Gael060515";

// Definindo os estados do sistema
enum AppState
{
  STATE_IDLE,
  STATE_LOGIN,
  STATE_MAIN_SCREEN
};

AppState app_state = STATE_IDLE; // Estado inicial

extern lv_event_t g_eez_event;
extern bool g_eez_event_login;

/*LVGL: Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

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
    data->state = LV_INDEV_STATE_PR;
  }
}

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
  Serial.begin(115200);
  setupWiFi(ssid, password);

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

  ui_init();

  Serial.println("Setup done");
  Serial.printf("Memória livre no heap: %d bytes\n", esp_get_free_heap_size());
  Serial.printf("Tamanho da pilha para a tarefa principal: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
}

void verifyMemory()
{
  Serial.print("Heap livre: ");
  Serial.println(esp_get_free_heap_size());
  delay(5000);
}

void loop()
{
  lv_timer_handler(); // Atualiza a interface LVGL
  ui_tick();          // Atualização personalizada, se necessário
  // verifyMemory();
  lv_obj_t *obj = nullptr; // Variável declarada fora do switch

  // Máquina de estados
  switch (app_state)
  {
  case STATE_IDLE:
    Serial.println("Waiting for login...");
    if (g_eez_event_login)
    {
      app_state = STATE_LOGIN;
    }
    break;

  case STATE_LOGIN:
    Serial.println("Logging in...");
    // Executa ações relacionadas ao login
    obj = lv_event_get_target(&g_eez_event); // Agora usamos a variável externa
    Serial.printf("Received event from obj: %u\n", obj);
    tft.fillScreen(TFT_RED);
    delay(1000);

    // Carrega a tela principal após o login
    lv_scr_load(objects.screen00);
    app_state = STATE_MAIN_SCREEN; // Vai para a próxima tela
    break;

  case STATE_MAIN_SCREEN:
    Serial.println("Main screen loaded.");
    // Aqui você pode adicionar lógica para detectar logout
    if (!g_eez_event_login)
    {
      app_state = STATE_IDLE;
      lv_scr_load(objects.main);
      g_eez_event_login = false; // Garante que não volte ao login automaticamente
    }
    break;
  }

  delay(100); // Evita sobrecarga no loop
}
