/**
 * @file    spi_slave_receive.c
 * @brief   ESP8266 SPI slave example to receive Ipeak, Irms, and Frequency
 * @author  Umesh
 * @date    2025-12-16
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi.h"
#include "driver/hspi_logic_layer.h"
#include "mqtt_publish.h"

#define BROKER "mqtt://broker.hivemq.com"
#define SSID "mutu_test"
#define PASSWORD "12a12@12"

#define SPI_SLAVE_HANDSHAKE_GPIO 2
#define SPI_READ_BUFFER_MAX_SIZE 12   // 6 bytes for 3 x uint16_t

static const char *TAG = "SPI_SLAVE";

// Variables to store received values
volatile uint16_t Ipeak = 0;
volatile uint16_t Irms  = 0;
volatile uint16_t Freq  = 0;

// SPI initialization
void comm_spi_init(void)
{
    spi_config_t spi_config;

    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    spi_config.intr_enable.val = SPI_SLAVE_DEFAULT_INTR_ENABLE;
    spi_config.mode = SPI_SLAVE_MODE;
    spi_config.event_cb = NULL;

    spi_init(HSPI_HOST, &spi_config);


    hspi_slave_logic_device_create(SPI_SLAVE_HANDSHAKE_GPIO, 1, SPI_READ_BUFFER_MAX_SIZE, SPI_READ_BUFFER_MAX_SIZE);
}





void IRAM_ATTR spi_slave_read_master_task(void *arg)
{
    uint8_t read_data[SPI_READ_BUFFER_MAX_SIZE];

    for (;;)
    {
        taskYIELD();

        int read_len = hspi_slave_logic_read_data(read_data, SPI_READ_BUFFER_MAX_SIZE, 1);

        if (read_len >= 7) { // 1 command + 6 bytes
    if (read_data[0] != 0x02) continue; // verify command

    Ipeak = (read_data[1] << 8) | read_data[2];
    Irms  = (read_data[3] << 8) | read_data[4];
    Freq  = (read_data[5] << 8) | read_data[6];

    ESP_LOGI(TAG, "Ipeak: %u, Irms: %u, Freq: %u", Ipeak, Irms, Freq);
    mqtt_publish_values(Ipeak, Irms, Freq);
}
    }}


void app_main(void)
{
    // Initialize SPI slave
    comm_spi_init();

    mqtt_init(SSID, PASSWORD, BROKER);

    xTaskCreate(spi_slave_read_master_task, "spi_slave_task", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "SPI Slave ready to  publish to MQTT...");
}
