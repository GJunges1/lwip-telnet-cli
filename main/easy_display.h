#define PIN_SDA 5 // SDA
#define PIN_SCL 4 // SCL
#define NUMBER_OF_LINES 6
#define TAG2 "ssd1306"

esp_err_t xDisplayInit(void);
esp_err_t xDisplayWrite(char* text);
esp_err_t xDisplayClear(void);