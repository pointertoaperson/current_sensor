#include "fir.h"

int16_t fir_buffer[FIR_TAPS] = {0}; 

// Q15 scaled symmetric coefficients (32 coefficients)
const int16_t fir_coeff[FIR_TAPS] = {
  -56, 64, -78, 86, -68, 0, 143, -377, 710, -1128,
  1605, -2105, 2573, -2952, 29530, 29530, -2952, 2573,
  -2105, 1605, -1128, 710, -377, 143, 0, -68, 86, -78,
  64, -56, 0, 0 
};


//  FIR filter function
fir_process(int16_t input)
{
    uint8_t fir_index = 0;
    fir_buffer[fir_index] = input;
    int32_t acc = 0;

    for (uint8_t i = 0; i < FIR_TAPS / 2; i++)
    {
        uint8_t a_idx = (fir_index + i) % FIR_TAPS;
        uint8_t b_idx = (fir_index + FIR_TAPS - 1 - i) % FIR_TAPS;
        acc += (int32_t)fir_coeff[i] * (fir_buffer[a_idx] + fir_buffer[b_idx]);
    }

    fir_index = (fir_index + 1) % FIR_TAPS;

    // Convert back to Q15
    acc = acc >> 15;

    // Saturation
    if (acc > 32767) acc = 32767;
    if (acc < -32768) acc = -32768;

    return (int16_t)acc;
}