//
//  main.m
//  cocoamemories
//
//  Simple mode 13h emulator using TIGR by Richard Mitton
//  TIGR available at https://bitbucket.org/rmitton/tigr/overview
//
//  Created by Virokannas, Simo on 5/17/20.
//

#import <Cocoa/Cocoa.h>
#include "tigr.h"
#include <time.h>

#include "vgapalette.h"

#include "timing.h"
#include "mainloop.h"

//#define FRAME_EXACT

int main(int argc, const char * argv[]) {
    init_clock_frequency();
    
    Tigr *screen = tigrWindow(320, 200, "Memories", 0);
    tigrClear(screen, tigrRGB(0x00, 0x00, 0x00));
    unsigned char *vga = malloc(320 * 300);
    unsigned char *padded_vga = vga + 320 * 50;
    memset(vga, 0, 320 * 300);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000 / 30;
    unsigned short bp = 0x13;
    float started = time_now();
    while (!tigrClosed(screen))
    {
        // convert vga to tigr
        TPixel *buf = screen->pix;
        unsigned char *v = padded_vga;
        for(int p=0;p<320*200;p++) {
            unsigned char c = *v;
            (*buf).b=vga_palette[c*3+2];
            (*buf).g=vga_palette[c*3+1];
            (*buf).r=vga_palette[c*3];
            ++v; ++buf;
        }
        advance(padded_vga, bp++);
        tigrUpdate(screen);
        // frame exact or timing exact?
#ifdef FRAME_EXACT
        float t = time_now() - started;
        if(t < 0.0333f) {
            ts.tv_nsec = t * 1000.0f;
            nanosleep(&ts, NULL);
        }
        started = time_now();
#else
        float t = time_now() - started;
        bp = (int)(t * 35.0f);
#endif
    }
    free(vga);
    tigrFree(screen);
    return 0;
}
