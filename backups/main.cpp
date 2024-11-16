#include <lvgl.h>
#include <TFT_eSPI.h>

#include <demos/lv_demos.h>

#include "touchpad.h"
#include "ui/ui.h"
#include <WiFi.h>
// #include <PubSubClient.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();
const char *ssid = "Tatakae";
const char *password = "Gael060515";
// const char* mqtt_server = "homeassistant.local";

WiFiClient espClient;
// PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

/**
 * WI-FI
 */
void setup_wifi()
{
  Serial.println("\nConectando ao Wi-Fi...");
  Serial.printf("Rede: %s\n", ssid);

  WiFi.begin(ssid, password);

  const unsigned long timeout = 20000; // Timeout de 20 segundos
  unsigned long startAttemptTime = millis();

  // Tenta conectar enquanto o timeout não for atingido
  while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < timeout)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConectado ao Wi-Fi!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFalha ao conectar ao Wi-Fi dentro do tempo limite.");
    // Aqui você pode decidir o que fazer:
    // - Continuar sem Wi-Fi
    // - Reiniciar o dispositivo
    // - Entrar em modo offline
  }
}

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
    Serial.println("Touch not detected. State set to LV_INDEV_STATE_REL.");
  }
  else
  {
    /* Set the coordinates */
    data->point.x = Dev_Now.X[0];
    data->point.y = Dev_Now.Y[0];
    data->state = LV_INDEV_STATE_PR;
    Serial.printf("Touch detected at X: %d, Y: %d\n", data->point.x, data->point.y);
  }
}

static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 20]; // Apenas 40 linhas por vez

/*LVGL: flush to display*/
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  Serial.printf("Flushing display, area width: %d, height: %d\n", w, h);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void setup()
{
  Serial.begin(115200);
  // setup_wifi();
  // client.setServer(mqtt_server, 1883);

  String LVGL_Arduino = "Hello Arduino!9999";
  Serial.println(LVGL_Arduino);
  delay(100);
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println("I am LVGL_Arduino");
  // GT911_Init_Touchpad();
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

  Serial.printf("Buffer de desenho alocado com %d bytes\n", screenWidth * 20 * sizeof(lv_color_t));

  // Verifique a quantidade de memória livre disponível
  Serial.printf("Memória livre no heap: %d bytes\n", esp_get_free_heap_size());

  // Inicialize o buffer de desenho
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 6);
  Serial.printf("Memória livre no heap após a alocação: %d bytes\n", esp_get_free_heap_size());

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
  delay(500);

}

void loop()
{
  Serial.println("Init loop");
  // Verificação de inicialização do LVGL antes de chamar o handler
  // Chama o handler LVGL para processar eventos e atualizar a interface
  lv_timer_handler(); // Atualiza a interface LVGL
  Serial.println("lv_timer_handler ok");

  // Atualização personalizada, se necessário

  // Máquina de estados
  switch (app_state)
  {
  case STATE_IDLE:
    Serial.println("Waiting for login...");
    if (g_eez_event_login)
    {
      app_state = STATE_LOGIN;
      Serial.println("State changed to STATE_LOGIN.");
    }
    break;

  case STATE_LOGIN:
    Serial.println("Logging in...");
    // Verifica o evento de login
    if (g_eez_event_login)
    {
      lv_obj_t *obj = lv_event_get_target(&g_eez_event); // Obtém o objeto que gerou o evento

      if (obj == nullptr)
      {
        Serial.println("Error: Object pointer is null in STATE_LOGIN.");
      }
      else
      {
        Serial.printf("Received event from object: %p\n", obj); // Log do objeto
      }
    }
    else
    {
      Serial.println("Error: g_eez_event is null in STATE_LOGIN.");
    }

    // Exemplo de ação de transição de tela
    tft.fillScreen(TFT_RED);
    delay(1000);

    // Carrega a tela principal após o login
    lv_scr_load(objects.screen00);
    app_state = STATE_MAIN_SCREEN; // Vai para a próxima tela
    Serial.println("State changed to STATE_MAIN_SCREEN.");
    break;

  case STATE_MAIN_SCREEN:
    Serial.println("Main screen loaded.");
    // Lógica para detectar logout (ou outras mudanças)
    if (!g_eez_event_login)
    {
      app_state = STATE_IDLE;
      lv_scr_load(objects.main);
      g_eez_event_login = false; // Garante que não volte ao login automaticamente
      Serial.println("State changed to STATE_IDLE.");
    }
    break;
  }

  // Controle do tempo com millis(), substituindo delay
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  // Realiza verificações periódicas
  if (currentMillis - lastMillis >= 1000)
  { // Executa a cada 1 segundo
    // Coloque aqui qualquer verificação periódica que você queira fazer
    // Exemplo: verificar a memória ou o status do sistema
    Serial.printf("Loop executed. Free heap: %d bytes\n", esp_get_free_heap_size());

    lastMillis = currentMillis;
  }
}