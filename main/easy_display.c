#include "u8g2_esp32_hal.h"
#include <u8g2.h>
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"

#include "easy_display.h"

//Component u8g2 functions:

static u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
static u8g2_t u8g2; // a structure which will contain all the data for one display
static SemaphoreHandle_t my_mutex = NULL;
static int idx_line=1;

esp_err_t xDisplayInit(void)
{
    u8g2_esp32_hal.sda  = PIN_SDA;
    u8g2_esp32_hal.scl  = PIN_SCL;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
            &u8g2,
            U8G2_R0,
            u8g2_esp32_i2c_byte_cb,
            u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
    u8x8_SetI2CAddress(&u8g2.u8x8,0x78);

    ESP_LOGI(TAG2, "u8g2_InitDisplay");
    u8g2_InitDisplay(&u8g2); // envia sequência de inicialização para o display

    ESP_LOGI(TAG2, "u8g2_SetPowerSave");
    u8g2_SetPowerSave(&u8g2, 0); // acorda o display
    ESP_LOGI(TAG2, "u8g2_ClearBuffer");
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    my_mutex = xSemaphoreCreateMutex();
    if(my_mutex==NULL)
    {
        return ESP_FAIL;
    }
    idx_line = 1;
    ESP_LOGI(TAG2, "All done!");
    return ESP_OK;
}

esp_err_t xDisplayWrite(char* text)
{
    xSemaphoreTake(my_mutex,portMAX_DELAY);
    if(idx_line > NUMBER_OF_LINES)
    {
        return ESP_FAIL;
    }
    int ret = u8g2_DrawStr(&u8g2,
        2, //x
        10*idx_line, //y
        text); //text to be displayed
    if(ret<0)
    {
        return ESP_FAIL;
    }

    u8g2_SendBuffer(&u8g2);
    idx_line=idx_line+1;
    xSemaphoreGive(my_mutex);
    return ESP_OK;
}

esp_err_t xDisplayClear(void)
{
    u8g2_ClearBuffer(&u8g2);
    idx_line = 1;
    return ESP_OK;
}