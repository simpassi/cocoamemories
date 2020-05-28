//
//  timing.c
//  cocoamemories
//
//

#include "timing.h"

// https://stackoverflow.com/questions/361363/how-to-measure-time-in-milliseconds-using-ansi-c

#include <mach/mach_time.h>
static uint64_t freq_num   = 0;
static uint64_t freq_denom = 0;

uint64_t global_clock = 0;

void init_clock_frequency () {
    mach_timebase_info_data_t tb;
    if (mach_timebase_info (&tb) == KERN_SUCCESS && tb.denom != 0) {
        freq_num   = (uint64_t) tb.numer;
        freq_denom = (uint64_t) tb.denom;
    }
    global_clock = mach_absolute_time();
}

float time_now() {
    uint64_t tick_value = mach_absolute_time ();
    uint64_t diff = tick_value - global_clock;
    diff /= 1000000;
    diff *= freq_num;
    diff /= freq_denom;
    return (float)diff / 1000.0f;
}
