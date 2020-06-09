//
//  cmemories.c
//  cocoamemories
//
//  Created by Virokannas, Simo on 6/1/20.
//  Copyright © 2020 IHIHI. All rights reserved.
//

#include "cmemories.h"
#include <stdint.h>

#include "x86.h"

// scrolling tilted plane
uint8_t cfx0(uint8_t x, uint8_t y, uint16_t t) { return (((((int8_t)x)*0x13)/(y+0x29))^(0x1329/(y+0x29)-t))&(28); }

// zooming circles
uint8_t cfx1(uint8_t x, uint8_t y, uint16_t t) { return (((y-100)*(y-100)+((int8_t)x*(int8_t)x))>>8)+t&24; }

// array of chessboards
uint8_t cfx2(uint8_t x, uint8_t y, uint16_t t) { return (((y-(t<<8))^(x-t))|0xdb)+0x13; }

// parallax checkerboards
uint8_t cfx3(uint8_t x, uint8_t y, uint16_t t) { int8_t b; uint16_t d; for(b=-16;b<0;++b) { t+=y*320+x; d=((int16_t)t*819)>>16; if(d&0x01){return b+32;} d=((d&0x01<<15))|(d>>1); } return b+31; }

uint8_t cfx4(uint8_t x, uint8_t y, uint16_t t) { return 0; }
uint8_t cfx5(uint8_t x, uint8_t y, uint16_t t) { return 0; }
uint8_t cfx6(uint8_t x, uint8_t y, uint16_t t) { return 0; }

uint8_t cdummy(uint8_t x, uint8_t y, uint16_t t) { return 0; }

uint8_t cstop(uint8_t x, uint8_t y, uint16_t t) { exit(0); }

// effects table
uint8_t (*ctb[])(uint8_t, uint8_t, uint16_t)= {cfx2, cfx1, cfx0, cfx3, cfx4, cfx5, cfx6, cdummy, cstop}; //{fx2,fx1,fx0,fx3,fx4,fx5,fx6,dummy,stop};

void c_advance(unsigned char *a000, unsigned short bp) {
    // inject our fake time pointer
    uint16_t i = 0;
    uint8_t p[4];
    uint32_t *res = (uint32_t *)p;
    uint16_t *pt = (uint16_t *)p;
    do {
        *res = (uint32_t)(0xcccd) * (uint32_t)i;
        p[0]+=p[1];p[1]^=p[1];pt[0]+=bp;pt[0]>>=9;p[1]&=15;
        a000[i++ - 160] = (*ctb[p[0]])(p[2],p[3],bp);
        i+=2;
    } while (i != 0);
}
