
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssd1306.h"
#include "fir.h"
#include "spi.h"
#include <math.h>

#define F_CPU 16000000UL
#define SAMPLE_RATE_HZ 1000
#define SAMPLE_COUNT 100

volatile uint16_t samples[SAMPLE_COUNT];
volatile uint8_t sample_idx = 0;
volatile uint8_t samples_ready = 0;

// --- Integration buffer ---
int16_t integrated[SAMPLE_COUNT];  // store scaled integrated samples in int16_t

void timer1_init_1khz(void)
{
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
    OCR1A = (F_CPU / (64UL * SAMPLE_RATE_HZ)) - 1;
    TIFR1 = (1 << OCF1A);
    TIMSK1 |= (1 << OCIE1A);
}

void adc_init(void)
{
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

ISR(TIMER1_COMPA_vect)
{
    ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect)
{
    if (sample_idx < SAMPLE_COUNT)
    {
        samples[sample_idx++] = ADC;
        if (sample_idx >= SAMPLE_COUNT)
        {
            samples_ready = 1;
            TIMSK1 &= ~(1 << OCIE1A);
            ADCSRA &= ~(1 << ADEN);
        }
    }
}

// --- Frequency estimation from filtered samples ---
float estimate_frequency(int16_t *data, uint8_t count)
{
    uint8_t last = (data[0] >= 0);
    uint16_t crossings = 0;

    for (uint8_t i = 1; i < count; i++)
    {
        uint8_t curr = (data[i] >= 0);
        if (curr != last)
            crossings++;
        last = curr;
    }

    float freq = ((float)crossings / 2.0f) * ((float)SAMPLE_RATE_HZ / (float)count);
    return freq;
}

int main(void)
{
    DDRC &= ~(1 << PC0);
    ssd1306_init();
    ssd1306_clear();
    ssd1306_update();
    adc_init();
    spi_init();
    sei();

    // --- Fixed-point integration scaling ---
    const int32_t dt_scaled = 327; // integration increment scaling
    const uint8_t dt_shift = 10;   // right shift to fit int16_t safely

    // --- Scaling factor for display ---
    const float scale = 5.0f / 32768.0f * (1 << dt_shift); // account for Q15 and shift

    while (1)
    {
        sample_idx = 0;
        samples_ready = 0;

        ADCSRA |= (1 << ADEN) | (1 << ADIE);
        ADCSRA |= (1 << ADIF);

        TIFR1 = (1 << OCF1A);
        timer1_init_1khz();

        while (!samples_ready) { }

        // --- Apply FIR filter and fixed-point integration ---
        int32_t y_acc = 0;   // 32-bit accumulator

        for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        {
            int16_t centered = samples[i] - 512;        // center ADC to 0
            int16_t filtered = fir_process(centered);   // Q15 filtered sample

            // Fixed-point integration: y[n] = y[n-1] + x[n]*dt_scaled
            y_acc += (int32_t)filtered * dt_scaled;

            // Scale down to fit int16_t safely
            integrated[i] = (int16_t)(y_acc >> dt_shift);

            samples[i] = filtered; // store filtered sample back if needed
        }

        // --- Compute peak and RMS from integrated array ---
        int16_t max_integrated = 0;
        int32_t sum_sq_integrated = 0;

        for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        {
            int16_t val = integrated[i];
            if (val < 0) val = -val;  // absolute value

            if (val > max_integrated) max_integrated = val;
            sum_sq_integrated += (int32_t)val * val;
        }

        float integrated_peak = max_integrated * scale;
        float integrated_rms  = sqrt((float)(sum_sq_integrated / SAMPLE_COUNT)) * scale;

        // --- Frequency from filtered samples ---
        float freq = estimate_frequency(samples, SAMPLE_COUNT);
        // --- Send data via SPI ---
    

        // --- Display ---
        char I_peak[6], I_rms[6], Freq[6], display[32];
        dtostrf(integrated_peak, 0, 2, I_peak);
        dtostrf(integrated_rms, 0, 2, I_rms);
        dtostrf(freq, 0, 1, Freq);

        snprintf(display, sizeof(display), "%s-%s", I_peak, I_rms);
        ssd1306_clear();
        ssd1306_draw_string_big(0, 0, "Ipeak    (mA)   Irms", 1);
        ssd1306_draw_string_big(0, 8, display, 2);
        snprintf(display, sizeof(display), "Freq: %s Hz", Freq);
        ssd1306_draw_string_big(0, 24, display, 1);
        ssd1306_update();
            spi_send_current((uint16_t)(integrated_peak * 100.0f) ,(uint16_t)(integrated_rms  * 100.0f), (uint16_t)(freq * 10.0f));

        _delay_ms(200);
    }
}


