#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssd1306.h"

#define F_CPU 16000000UL
#define SAMPLE_RATE_HZ 1000
#define SAMPLE_COUNT 100
#define SCALE_FACTOR 37.40f // current scaling
volatile uint16_t samples[SAMPLE_COUNT];
volatile uint8_t sample_idx = 0;
volatile uint8_t samples_ready = 0;

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

float estimate_frequency(uint16_t *data, uint8_t count, float vref)
{
    uint16_t midpoint = 512;
    uint8_t last = (data[0] >= midpoint);
    uint16_t crossings = 0;

    for (uint8_t i = 1; i < count; i++)
    {
        uint8_t curr = (data[i] >= midpoint);
        if (curr != last)
            crossings++;
        last = curr;
    }

    float freq = ((float)crossings / 2.0f) * ((float)SAMPLE_RATE_HZ / (float)count);
    return freq;
}

#define PEAK_AVG_COUNT 20
float peak_buffer[PEAK_AVG_COUNT];
uint8_t peak_index = 0;
uint8_t peak_filled = 0;

// --- Current smoothing buffer ---
#define CURRENT_AVG_COUNT 5
float current_buffer[CURRENT_AVG_COUNT];
uint8_t current_index = 0;
uint8_t current_filled = 0;

// --- Simple moving average filter for samples ---
#define MA_WINDOW 5
void apply_moving_average(uint16_t *data, uint8_t count)
{
    uint16_t temp[SAMPLE_COUNT];
    for (uint8_t i = 0; i < count; i++)
        temp[i] = data[i];

    for (uint8_t i = 0; i < count; i++)
    {
        uint32_t sum = 0;
        uint8_t w_start = (i < MA_WINDOW) ? 0 : i - MA_WINDOW + 1;
        uint8_t w_end = i;
        uint8_t w_len = w_end - w_start + 1;
        for (uint8_t j = w_start; j <= w_end; j++)
            sum += temp[j];
        data[i] = sum / w_len;
    }
}

int main(void)
{
    DDRC &= ~(1 << PC0);
    ssd1306_init();
    ssd1306_clear();
    ssd1306_update();
    adc_init();
    sei();

    while (1)
    {
        sample_idx = 0;
        samples_ready = 0;

        ADCSRA |= (1 << ADEN) | (1 << ADIE);
        ADCSRA |= (1 << ADIF);

        TIFR1 = (1 << OCF1A);
        timer1_init_1khz();

        while (!samples_ready)
        {
        }

        // --- Estimate frequency ---
        float freq = estimate_frequency(samples, SAMPLE_COUNT, 5.0f);

        // --- Apply filter to samples ---
        apply_moving_average(samples, SAMPLE_COUNT);

        // --- Compute peak and RMS ---
        uint16_t maxv = 0;
        uint32_t sum_sq = 0;

        for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        {
            int16_t centered = samples[i] - 512;
            samples[i] = (centered > 0) ? centered : 0;
        }

        for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
        {
            if (samples[i] > maxv)
                maxv = samples[i];
            sum_sq += ((int32_t)samples[i] * samples[i]);
        }

        float current = ((float)maxv * (5.0f / 1023.0f));
        //float current_rms = (((float)sqrt(sum_sq / SAMPLE_COUNT)) * (5.0f / 1023.0f));

        // --- Smooth the current using circular buffer ---
        current_buffer[current_index++] = current;
        if (current_index >= CURRENT_AVG_COUNT)
        {
            current_index = 0;
            current_filled = 1;
        }

        float smoothed_current = 0;
        uint8_t c_count = current_filled ? CURRENT_AVG_COUNT : current_index;
        for (uint8_t i = 0; i < c_count; i++)
            smoothed_current += current_buffer[i];
        smoothed_current /= c_count;

        // --- Add smoothed current to peak buffer ---
        peak_buffer[peak_index++] = smoothed_current;
        if (peak_index >= PEAK_AVG_COUNT)
        {
            peak_index = 0;
            peak_filled = 1;
        }

        // --- Compute average peak ---
        float avg_peak = 0;
        uint8_t count = peak_filled ? PEAK_AVG_COUNT : peak_index;
        for (uint8_t i = 0; i < count; i++)
            avg_peak += peak_buffer[i];
        avg_peak /= count;


        float current_rms =  avg_peak * 71.428f;//(((float)sqrt(sum_sq / SAMPLE_COUNT)) * (5.0f / 1023.0f));
        avg_peak =  current_rms* (0.707);//((float)maxv * (5.0f / 1023.0f));
        char I_peak[6], I_rms[6], Freq[6], display[32];
        dtostrf(avg_peak, 0, 1, I_peak);
        dtostrf(current_rms, 0, 1, I_rms);
        dtostrf(freq, 0, 1, Freq);

        snprintf(display, sizeof(display), "%s-%s", I_peak, I_rms);

        ssd1306_clear();
        ssd1306_draw_string_big(0, 0, "Ipeak    (mA)   Irms", 1);
        ssd1306_draw_string_big(0, 8, display, 2);
        snprintf(display, sizeof(display), "    Freq: %sHz", Freq);
        ssd1306_draw_string_big(0, 24, display, 1);
        ssd1306_update();

        _delay_ms(200);
    }
}
