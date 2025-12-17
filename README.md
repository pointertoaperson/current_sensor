![project](/rogowoski_project.png)
# Rogowski Coil–Based Current Sensor with Edge Connectivity

This project implements a **non-invasive current measurement system** based on a **Rogowski coil**, combined with embedded digital signal processing and wireless data transmission. The system measures **peak current, RMS current, and frequency** of an AC signal and publishes the results to an **MQTT broker** for real-time edge visualization.

The project demonstrates a complete **cyber-physical system (CPS)** pipeline, spanning analog sensing, bare-metal firmware, digital signal processing, wireless communication, and edge-side visualization.

---

## Key Features

- Non-invasive current sensing using a **hand-wound Rogowski coil**
- Wide bandwidth measurement without magnetic saturation
- **Bare-metal firmware** on ATmega328P (no Arduino / RTOS)
- Fixed-point **FIR filtering** and **digital integration**
- Peak, RMS, and frequency estimation
- Real-time OLED display (SSD1306)
- **SPI communication** between ATmega328P and ESP8266
- **Wi-Fi + MQTT** data publishing using ESP8266 (ESP_RTOS_SDK)
- **Edge visualization** using Python and Matplotlib

---
### visualization app 
 - edge_listener.py
 
## System Architecture

### Hardware Blocks

- **Rogowski Coil + Signal Conditioning**  
  Produces a small voltage proportional to `di/dt`, followed by amplification and conditioning.

- **ATmega328P (16 MHz)**  
  Performs ADC sampling, FIR filtering, digital integration, current calculation, frequency estimation, and display control.

- **SSD1306 OLED (128×32, I²C)**  
  Displays peak current, RMS current, and frequency locally.

- **ESP8266**  
  Acts as an SPI slave, connects to Wi-Fi, and publishes data via MQTT.

- **Edge Device (PC)**  
  Subscribes to MQTT data and visualizes it in real time using Python.

---

## Firmware Overview

### ATmega328P (Bare-Metal)

- Written in **register-level C** for deterministic behavior
- No Arduino framework or operating system
- Timer-driven ADC sampling at **1 kHz**
- 31-tap **FIR low-pass filter** implemented using **Q15 fixed-point arithmetic**
- Digital integration using Forward Euler method
- Zero-crossing based frequency estimation
- SPI master for data transfer to ESP8266

Source files:

- `main.c` – System initialization and main loop  
- `adc.c / adc.h` – ADC configuration and sampling  
- `timer.c / timer.h` – Timer-based sampling control  
- `fir.c / fir.h` – FIR filter implementation  
- `spi.c / spi.h` – SPI communication with ESP8266  
- `i2c.c / i2c.h` – I²C driver  
- `ssd1306.c / ssd1306.h` – OLED driver  

---

### ESP8266 (ESP_RTOS_SDK)

- Configured as **SPI slave**
- Receives current and frequency values from ATmega328P
- Connects to Wi-Fi network
- Publishes measurements via **MQTT** in **JSON format**


