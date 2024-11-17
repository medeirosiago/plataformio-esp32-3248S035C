#include <lvgl.h>
#include <TFT_eSPI.h>

#include "touchpad.h"
#include "ui/ui.h"
#include "ui/vars.h"

#include "utils/mqtt/MqttHandler.h"

// Variáveis globais para armazenar os valores
static float air_temperature = 0.0;
static float room_temperature = 0.0;

float get_var_air_temperature() {
    return air_temperature;
}

void set_var_air_temperature(float value) {
    air_temperature = value;
    // Atualizar a interface do LVGL com o novo valor
    lv_event_send(objects.air_temperature, LV_EVENT_VALUE_CHANGED, &air_temperature);
}

float get_var_room_temperature() {
    return room_temperature;
}

void set_var_room_temperature(float value) {
    room_temperature = value;
    // Atualizar a interface do LVGL com o novo valor
    lv_event_send(objects.air_temperature, LV_EVENT_VALUE_CHANGED, &room_temperature);
}

/**
 * MQTT Settings
 */
const char *wifiSsid = "Tatakae";
const char *wifiPassword = "Gael060515";
IPAddress mqttHost(192, 168, 0, 123); // Altere pelo IP do seu broker MQTT
uint16_t mqttPort = 1883;             // Porta padrão MQTT
const char *mqttUsername = "magoia";
const char *mqttPassword = "8191323@@Magoiamon";
const char *mqttTopic = "home/ar/status";
uint8_t qos = 1;

MqttHandler mqttHandler(wifiSsid, wifiPassword, mqttHost, mqttPort, mqttUsername, mqttPassword, mqttTopic, qos);

TFT_eSPI tft = TFT_eSPI();

// Definindo os estados do sistema
enum AppState {
    STATE_IDLE,
    STATE_LOGIN,
    STATE_MAIN_SCREEN
};

AppState app_state = STATE_IDLE; // Estado inicial

extern lv_event_t g_eez_event;
extern bool g_eez_event_login;

/*LVGL: Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;

    GT911_Scan();
    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
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

/*LVGL: flush to display*/
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void setup() {
    Serial.begin(115200);

    // y = touch.Y();
    String LVGL_Arduino = "Hello Arduino!9999";
    Serial.println(LVGL_Arduino);
    delay(100);
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println("I am LVGL_Arduino");
    mqttHandler.begin();
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
}

void loop() {
    lv_timer_handler();      // Atualiza a interface LVGL
    ui_tick();               // Atualização personalizada, se necessário
    lv_obj_t *obj = nullptr; // Variável declarada fora do switch

    // Checar o status climático e atualizar variáveis globais
    ClimateStatus currentStatus = mqttHandler.getClimateStatus();
    if (currentStatus.valid) {
        // Atualizar as variáveis globais com os valores recebidos
        set_var_air_temperature(currentStatus.setTemperature);
        set_var_room_temperature(currentStatus.currentTemperature);

        // Imprimir os valores recebidos para debug
        Serial.printf("Power: %s, Set Temp: %.1f, Current Temp: %.1f, Fan Mode: %s, Tornado: %s\n",
                      currentStatus.power.c_str(),
                      currentStatus.setTemperature,
                      currentStatus.currentTemperature,
                      currentStatus.fanMode.c_str(),
                      currentStatus.tornado ? "true" : "false");
    } else {
        Serial.println("Dados climáticos inválidos ou não recebidos.");
    }

    switch (app_state) {
    case STATE_IDLE:
        if (g_eez_event_login) {
            app_state = STATE_LOGIN;
        }
        break;

    case STATE_LOGIN:
        // Serial.println("Logging with");

        // // Executa ações relacionadas ao login
        // obj = lv_event_get_target(&g_eez_event); // Agora usamos a variável externa
        // Serial.printf("Received event from obj: %u\n", obj);
        // tft.fillScreen(TFT_RED);
        // delay(1000);

        // Carrega a tela principal após o login
        lv_scr_load(objects.air_temperature);

        break;

    case STATE_MAIN_SCREEN:
        // Aqui você pode adicionar lógica para detectar logout
        if (!g_eez_event_login) {
            app_state = STATE_IDLE;
            lv_scr_load(objects.main);
            g_eez_event_login = false; // Garante que não volte ao login automaticamente
        }
        break;
    }

    delay(100); // Evita sobrecarga no loop
}
