#ifndef _FIR_H
#define _FIR_H

#include <stdint.h>


#define FIR_TAPS 32


int16_t fir_process(int16_t input);

#endif