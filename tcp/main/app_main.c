#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "gpio_setup.h"
#include "adc_module.h"
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "wifi.h"
#include "mqtt.h"

#define sensorPin 5

int sensorValue = 0;
SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

void liga_led_vermelho(void){
    int LED = 4;
    esp_rom_gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    int estado = 1;
    gpio_set_level(LED, estado);
}

void liga_led_verde(void){
    int LED = 23;
    esp_rom_gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    int estado = 1;
    gpio_set_level(LED, estado);
}

void desliga_led_verde(void){
    int LED = 23;
    esp_rom_gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    int estado = 0;
    gpio_set_level(LED, estado);
}

void desliga_led_vermelho(void){
    int LED = 4;
    esp_rom_gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    int estado = 0;
    gpio_set_level(LED, estado);
}

void conectadoWifi(void * params)
{
  while(true)
  {
    if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      mqtt_start();
    }
  }
}

void envia_mensagem_presenca()
{
    
}

void envia_mensagem_nao_presenca()
{

}

void trataComunicacaoComServidor(void * params)
{
    char mensagem[50];
    char presenca[50];
    if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
    {
        while (true)
        {
            sensorValue = digitalRead(sensorPin);
            if (sensorValue < 1)
            {
                desliga_led_verde();
                liga_led_vermelho();
                bool led_vermelho = true;
                bool led_verde = false;
                sprintf(mensagem, "{\"estado led verde\": %d, \"estado led vermelho\": %d}", led_verde, led_vermelho);
                sprintf(presenca, "{\"tem presenca\": %d}", led_vermelho);
                mqtt_envia_mensagem("v1/devices/me/attributes", mensagem);
                mqtt_envia_mensagem("v1/devices/me/attributes", presenca);
                vTaskDelay(1000 / portTICK_PERIOD_MS);   
            }else{
                desliga_led_vermelho();
                liga_led_verde();
                    bool led_verde = true;
                    bool led_vermelho = false;
                    sprintf(mensagem, "{\"estado led verde\": %d, \"estado led vermelho\": %d}", led_verde, led_vermelho);
                    sprintf(presenca, "{\"tem presenca\": %d}", led_vermelho);
                    mqtt_envia_mensagem("v1/devices/me/attributes", mensagem);
                    mqtt_envia_mensagem("v1/devices/me/attributes", presenca);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
  }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    adc_init(ADC_UNIT_1);
    pinMode(sensorPin, GPIO_INPUT_PULLDOWN);

    xTaskCreate(&conectadoWifi,  "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);

}