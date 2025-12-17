# ESP8266 – SPI to Wi-Fi MQTT Bridge

This module implements the communication and networking layer of the system. The ESP8266 is configured as an SPI slave to the ATmega328P, receives processed measurement data, and publishes it to an MQTT broker over Wi-Fi. It acts as a bridge between the embedded sensor node and edge or cloud-based applications.

## Functionality

- SPI slave communication with ATmega328P  
- Wi-Fi connectivity in station (STA) mode  
- MQTT-based data publishing  
- JSON-formatted sensor data payload  

## Data Flow

ATmega328P → SPI → ESP8266 → Wi-Fi → MQTT Broker → Edge Device

## SPI Interface

The ATmega328P operates as the SPI master, while the ESP8266 functions as the SPI slave. Data is transferred using the following packet format:

[Command] [Sub-command] [Ipeak (2 bytes)] [Irms (2 bytes)] [Freq (2 bytes)]

The command byte `0x02` indicates the start of a valid transmission. Sensor values are transmitted as scaled integers to preserve precision.

## Wi-Fi and MQTT Communication

The ESP8266 connects to a configured Wi-Fi network and initializes the MQTT client only after a successful connection. Sensor data is published to the public MQTT broker `broker.hivemq.com` under the topic:

/esp8266/sensor

The payload is formatted as JSON, for example:

{ "Ipeak": 2.34, "Irms": 1.56, "Freq": 50.12 }

This format allows easy parsing and real-time processing by edge devices.

## Software Framework

The firmware is developed using the ESP8266_RTOS_SDK, an RTOS-based framework provided by Espressif and similar in structure to ESP-IDF.

https://github.com/espressif/ESP8266_RTOS_SDK

## Purpose

By offloading Wi-Fi and MQTT communication to the ESP8266, the ATmega328P can focus on real-time signal processing while enabling IoT connectivity and edge-level visualization.

## License

This module is intended for educational and research purposes and may be freely modified and extended.
