#include "mqtt_publish.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;
static bool mqtt_connected = false;

// Event group for Wi-Fi connection
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// --- Wi-Fi event handler ---
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected. Reconnecting...");
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

// --- MQTT event handler ---
static esp_err_t mqtt_event_handler_cb(void *handler_args, esp_event_base_t base,
                                       int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    if (!event) return ESP_FAIL;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            mqtt_connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            mqtt_connected = false;
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Message published");
            break;
        default:
            break;
    }
    return ESP_OK;
}

// --- Start MQTT client ---
static void mqtt_app_start(const char* mqtt_uri)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = mqtt_uri,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(client);
}

// --- Initialize Wi-Fi and MQTT ---
void mqtt_init(const char* ssid, const char* password, const char* mqtt_uri)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    // Wi-Fi configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
    snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID: %s ...", ssid);

    // Wait for Wi-Fi connection
    wifi_event_group = xEventGroupCreate();
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdTRUE,
                                           pdMS_TO_TICKS(15000)); // wait up to 15 sec

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connected successfully, starting MQTT...");
        // Start MQTT only after Wi-Fi is connected
        mqtt_app_start(mqtt_uri);
    } else {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi, MQTT not started");
    }
}

// Publish JSON values 
void mqtt_publish_values(uint16_t Ipeak, uint16_t Irms, uint16_t Freq)
{
    if (!mqtt_connected || !client) {
        ESP_LOGW(TAG, "MQTT not connected, skipping publish");
        return;
    }

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{ \"Ipeak\": %f, \"Irms\": %f, \"Freq\": %f }",
             Ipeak/100.0f, Irms/100.0f, Freq/100.0f);

    esp_mqtt_client_publish(client, "/esp8266/sensor", payload, 0, 1, 0);
    ESP_LOGI(TAG, "Published JSON: %s", payload);
}
