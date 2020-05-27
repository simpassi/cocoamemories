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

#include "mainloop.h"

int main(int argc, const char * argv[]) {
    Tigr *screen = tigrWindow(320, 200, "Memories", 0);
    tigrClear(screen, tigrRGB(0x00, 0x00, 0x00));
    unsigned char *vga = malloc(320 * 200);
    memset(vga, 0, 320 * 200);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000 / 30;
    unsigned short bp = 0x13;
    while (!tigrClosed(screen))
    {
        // convert vga to tigr
        TPixel *buf = screen->pix;
        unsigned char *v = vga;
        for(int p=0;p<320*200;p++) {
            unsigned char c = *v;
            (*buf).b=vga_palette[c*3+2];
            (*buf).g=vga_palette[c*3+1];
            (*buf).r=vga_palette[c*3];
            ++v; ++buf;
        }
        advance(vga, bp++);
        tigrUpdate(screen);
        nanosleep(&ts, NULL);
    }
    free(vga);
    tigrFree(screen);
    return 0;
}
