#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include <stdint.h>

// Initialize Wi-Fi and MQTT
void mqtt_init(const char* ssid, const char* password, const char* mqtt_uri);

// Publish values in JSON format
void mqtt_publish_values(uint16_t Ipeak, uint16_t Irms, uint16_t Freq);

#endif // MQTT_PUBLISH_H
